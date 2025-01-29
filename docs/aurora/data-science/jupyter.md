# Running JupyterLab and Jupyter Notebook on Aurora

ALCF provides a [JupyterHub](https://jupyter.alcf.anl.gov/) for running JupyterLab and Jupyter Notebook on Polaris and Sophia with minimal setup required from the user. While this service is not yet available on Aurora, users can still run JupyterLab and Notebook using SSH tunneling and port forwarding.

---

## Prerequisites
Please note that you only need a terminal (to SSH into Aurora) and a browser on your local machine (laptop or desktop). All the required packages need to be installed on Aurora only.

---

## 1. Initial Setup
1. **SSH into Aurora**:  
   The first step is to connect to Aurora through SSH. Note that tunneling or port forwarding is not required for this step.  
   If you have a problem with this step, please check [Getting Started on Aurora](../getting-started-on-aurora.md).
   ```bash
   ssh <your-username>@aurora.alcf.anl.gov
   ```

2. **Create a Virtual Environment**:  
   If you already have a Python environment, you can skip this step.
   ```bash
   python3 -m venv myenv
   ```
   Note that this command will create a directory called `myenv` in the current working directory, and `myenv` will also be the name of the virtual environment. You can change the name as you see fit.  
   You can find more information about creating a virtual environment on Aurora [here](./python.md).

3. **Install Required Packages**:  
   The third step is to install `jupyterlab` and/or `notebook` as well as `ipykernel`.  
   Note that you need to activate the virtual environment before installing the packages. Here we assume that the virtual environment is named `myenv` and is located in the current working directory.
   ```bash
   source myenv/bin/activate
   pip install jupyterlab notebook ipykernel
   ```

4. **Install IPython Kernel**:  
   The fourth step is to install the IPython kernel for the current virtual environment. IPython kernels enable easily switching between different Python environments on JupyterLab and Notebook.
   ```bash
   python -m ipykernel install --user --name myenv
   ```
   Steps 2, 3, and 4 are only required once for each virtual environment.

---

## 2. Run JupyterLab on a Login Node 

!!! warning
    
    This is not recommended for compute-intensive or memory-intensive workloads. Run the JupyterLab server on a compute node (see below section) if the workload is heavy.

1. **Start JupyterLab**:
   ```bash
   source myenv/bin/activate
   jupyter lab --no-browser --port=9999
   ```

2. **Copy the Address**:  
   The address will be displayed as the output of the previous command.
   ```
   http://127.0.0.1:9999/?token=<provided-token>
   ```

3. **Set Up SSH Tunneling**:  
   Open a new terminal tab or window on your local machine and run the following command:
   ```bash
   ssh -L 9999:127.0.0.1:9999 <your-username>@aurora.alcf.anl.gov
   ```
   - Replace `9999` with another port if it is unavailable.

4. **Access JupyterLab**:  
   Open your browser and navigate to the address copied above:
   ```
   http://localhost:9999/?token=<provided-token>
   ```

---

## 3. Run JupyterLab on a Compute Node

### Step 1: Request a Compute Node
You need a job running on Aurora to launch JupyterLab on a compute node. Below is an example of how to submit an interactive job to request a compute node. Note that you can also connect to one of the compute nodes of any of your non-interactive jobs that is running on Aurora. See [Running Jobs on Aurora](../running-jobs-aurora.md) for more information.

1. **Submit a Job**:  
   Submit an interactive job to request a compute node:
   ```bash
   qsub -l select=1 -l walltime=60:00 -A <project_name> -q <queue_name> -I
   ```
   You need to modify the `-A` and `-q` options to match your project name and queue name as well as the resources you need.

2. **Find Compute Node Hostname**:  
   Once the interactive job starts, you will be connected to the compute node. You can check the hostname with:
   ```bash
   hostname
   ```
   If you are not running an interactive job, you can find the hostname of the compute node by checking the `qstat` output.  
   First, you need to find the job ID of the job you are interested in. The following command will list all the jobs you have submitted. You need the ID of any one of the **running** jobs you are interested in.
   ```bash
   qstat -u <your_username>
   ```
   Then, we can find the hostname of the compute node by running:
   ```bash
   qstat -f <job_id>
   ```
   The hostname of the compute node will be displayed in the `exec_host` field. You can extract the hostname from the output with the following command:
   ```bash
   qstat -f <job_id> | awk -F '=' '/exec_host/ {print $2}' | tr '+' '\n' | cut -d '/' -f 1
   ```
   This should give you a hostname like `x4603c0s0b0n0`. Note that you can SSH into the compute nodes only when your job is running.

### Step 2: Start JupyterLab on the Compute Node
1. **SSH to the Compute Node**:  
   From the login node:
   ```bash
   ssh <compute_node_hostname>
   ```

2. **Activate the Environment**:
   ```bash
   source <path_to_your_virtual_environment>/bin/activate
   ```

3. **Start JupyterLab**:
   ```bash
   jupyter lab --no-browser --port=9999
   ```

### Step 3: Set Up SSH Tunneling
1. **Tunnel from Compute Node to Local Machine**:  
   On your local machine, run:
   ```bash
   ssh -L 9999:127.0.0.1:9999 -J <your-username>@aurora.alcf.anl.gov <your-username>@<compute_node_hostname>
   ```
   Please note that the `-J` option is used to specify the jump host, which is the Aurora login node.  
   Replace `9999` with another port if it is unavailable.

2. **Access JupyterLab**:  
   Open your browser and navigate to:
   ```
   http://localhost:9999/?token=<your-token>
   ```

---

!!! tip

    You can use `tmux` or `screen` to keep JupyterLab running if the SSH connection drops.