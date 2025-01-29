# Julia on Aurora

## Overview

Julia is a high-level, high-performance dynamic programming language for technical computing. It provides a sophisticated compiler, distributed parallel execution, numerical accuracy, and an extensive mathematical function library.

## Getting Started

To start using Julia on Aurora, follow these steps:

1. **Load the Julia module**: Use the module system to load Julia.
   ```bash
   module load julia
   ```

2. **Run Julia**: Start the Julia REPL by typing `julia` in the terminal.
   ```bash
   julia
   ```

3. **Install Packages**: Use Julia's package manager to install any required packages.
   ```julia
   using Pkg
   Pkg.add("PackageName")
   ```

## Example Code

Here is a simple example of a Julia script that calculates the factorial of a number:

```julia
function factorial(n::Int)
    if n == 0
        return 1
    else
        return n * factorial(n - 1)
    end
end

println(factorial(5))
```

## Resources

- [Julia Documentation](https://docs.julialang.org/)
- [JuliaLang GitHub](https://github.com/JuliaLang/julia)

## Support

For further assistance, please contact the support team at [support@aurora.com](mailto:support@aurora.com).