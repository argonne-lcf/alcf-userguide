# PyTorch Geometric (PyG)

[PyTorch Geometric (PyG)](https://pyg.org/) is a Python library built on top of PyTorch for deep learning on graphs. It provides tools for working with graph-structured data and implementations of many Graph Neural Networks (GNNs).

## PyG on Aurora

PyTorch Geometric includes a base library, called [`torch_geometric`](https://github.com/pyg-team/pytorch_geometric), and a number of optional dependencies:
- [`torch_scatter`](https://github.com/rusty1s/pytorch_scatter)
- [`torch_sparse`](https://github.com/rusty1s/pytorch_sparse)
- [`torch_cluster`](https://github.com/rusty1s/pytorch_cluster)
- [`torch_spline_conv`](https://github.com/rusty1s/pytorch_spline_conv)
- [`pyg_lib`](https://github.com/pyg-team/pyg-lib)

The [base library](#pyg-base-library), `torch_geometric`, relies solely on PyTorch and can utilize Intel GPUs through the `xpu` device specification.

The [optional dependencies](#optional-dependencies) include optimized operations that are relevant to GNNs and are missing in PyTorch. They have only CPU and CUDA implementations and can therefore only run on Aurora CPUs.

## PyG base library

Here we explain how to install the base library, `torch_geometric`, on Aurora and show a simple example of training a PyG model on Intel GPUs.

### Installing `torch_geometric` on Aurora

We are going to install `torch_geometric` in a [virtual environment](../python.md#virtual-environments-via-venv) that inherits PyTorch and the other libraries from the base frameworks module:

```bash
module load frameworks
python3 -m venv --clear venv --system-site-packages
source venv/bin/activate
pip install torch_geometric
```

### Example

The following example is inspired by the [`gcn.py` example](https://github.com/pyg-team/pytorch_geometric/blob/master/examples/gcn.py) on the PyG repository.

- Copy the following Python script into a file called `gcn.py`.
  ```python
  # gcn.py
  import argparse
  import time
  import torch
  import intel_extension_for_pytorch as ipex
  import torch_geometric
  
  parser = argparse.ArgumentParser()
  parser.add_argument('--dataset', type=str, default='Cora')
  parser.add_argument('--hidden_channels', type=int, default=256)
  parser.add_argument('--lr', type=float, default=0.01)
  parser.add_argument('--epochs', type=int, default=20)
  parser.add_argument('--device', choices=['auto', 'cpu'], default='auto', help='device to use')
  parser.add_argument('--num_nodes', type=int, default=10000)
  parser.add_argument('--num_edges', type=int, default=5000000)
  parser.add_argument('--num_features', type=int, default=32)
  parser.add_argument('--num_classes', type=int, default=100)
  args = parser.parse_args()
  
  device = torch.device(args.device)
  print(f"device: {device}")
  
  edge_index = torch.randint(args.num_nodes, size=(args.num_edges, 2), dtype=torch.long)
  x = torch.randn(size=(args.num_nodes, args.num_features))
  y = torch.randint(args.num_classes, size=(args.num_nodes,), dtype=torch.long)
  data = torch_geometric.data.Data(x=x, edge_index=edge_index.t().contiguous(), y=y, num_classes=(y.max()+1).item())
  data = data.to(device)
  
  class GCN(torch.nn.Module):
      def __init__(self, in_channels, hidden_channels, out_channels):
          super().__init__()
          self.conv1 = torch_geometric.nn.GCNConv(in_channels, hidden_channels)
          self.conv2 = torch_geometric.nn.GCNConv(hidden_channels, out_channels)
  
      def forward(self, x, edge_index, edge_weight=None):
          x = torch.nn.functional.dropout(x, p=0.5, training=self.training)
          x = self.conv1(x, edge_index, edge_weight).relu()
          x = torch.nn.functional.dropout(x, p=0.5, training=self.training)
          x = self.conv2(x, edge_index, edge_weight)
          return x
  
  model = GCN(
      in_channels=data.num_features,
      hidden_channels=args.hidden_channels,
      out_channels=data.num_classes,
  ).to(device)
  
  optimizer = torch.optim.Adam([
      dict(params=model.conv1.parameters(), weight_decay=5e-4),
      dict(params=model.conv2.parameters(), weight_decay=0)
  ], lr=args.lr)  # Only perform weight-decay on first convolution.
  model, optimizer = ipex.optimize(model, optimizer=optimizer)
  
  def train():
      model.train()
      optimizer.zero_grad()
      out = model(data.x, data.edge_index, data.edge_attr)
      loss = torch.nn.functional.cross_entropy(out, data.y)
      loss.backward()
      optimizer.step()
      return float(loss)
  
  times = []
  for epoch in range(1, args.epochs + 1):
      start = time.time()
      loss = train()
      times.append(time.time() - start)
  print(f'Median time per epoch: {torch.tensor(times).median():.4f}s')
  ```
- [Start an interactive job](../../running-jobs-aurora.md#submitting-a-job) on one node.
- Load the frameworks module, activate the virtual environment, and run the script:
  ```bash
  module load frameworks
  source venv/bin/activate
  python gcn.py
  ```
  The output should look like:
  ```bash
  device: xpu
  Median time per epoch: 0.2721s
  ```
- To run on the CPU, use `python gcn.py --device cpu`.

## Optional dependencies

Use the following script to create a virtual environment and install the base library and the CPU versions of all the optional dependencies:

```bash
#!/bin/bash 

module load frameworks

TORCH_LIB=$(python -c "import torch; print(torch.__file__)" | sed 's/__init__.py/lib/')
TORCH_VERSION=`python -c "import torch; print(torch.__version__)" | sed 's/^\([0-9.]*\).*/\1/'`

python3 -m venv --clear venv --system-site-packages
source venv/bin/activate

export LD_LIBRARY_PATH=${TORCH_LIB}:$LD_LIBRARY_PATH

# PyTorch Geometric and utils
pip install torch_geometric

libs=(\
"https://github.com/rusty1s/pytorch_scatter.git" \
"https://github.com/rusty1s/pytorch_sparse.git" \
"https://github.com/rusty1s/pytorch_cluster.git" \
"https://github.com/rusty1s/pytorch_spline_conv.git")

for lib in "${libs[@]}"; do
    LIB_NAME="$(basename "$lib" .git)"
    git clone ${lib} && cd "$LIB_NAME"
    git submodule update --init --recursive
    # get library version compatible with pytorch
    LIB_VERSION=`wget -O - https://data.pyg.org/whl/torch-${TORCH_VERSION}%2Bcpu.html 2>/dev/null | \
        grep -m1 $(echo ${LIB_NAME} | sed 's/pytorch_\(.*\)/\1/') | \
        sed 's/.*torch_[a-z_]*-\([^+-]*\)[%+-].*/\1/'`
    git checkout ${LIB_VERSION} 
    # Force to install without OpenMP backend
    sed "s|if ('backend: OpenMP' in info and 'OpenMP not found' not in info|if (False|g" setup.py > tmp && \
        mv tmp setup.py
    pip install .
    cd ..
    rm -rf "$LIB_NAME"
done
```