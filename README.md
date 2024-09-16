This is a custom shell program, designed to function similarly to popular Unix shells like Bourne Shell (bash). It parses user input and creates forked child processes to execute commands.

### Key Features:
- Process creation and management using `fork()`
- Support for basic shell commands (`ls`, `cd`, `rmdir`, `mkdir`, `rm`, `touch`, `cp`, `history`, `exit`, `quit`)
- Command history with recall (up to 15 commands), including PID tracking.
- Continuous command execution until the user exits

## Getting Started

### Prerequisites

- GCC (GNU Compiler Collection)
- Unix-like operating system
  
### Building

```bash
make
```

### Running

To start the shell, run the compiled binary:

```bash
./msh
```

## Usage

Once the shell is running, you can enter commands as you would in a regular Unix shell. Here are some examples:

```bash
history            # List command history
history -p         # List command history PIDs
!n                 # Re-run the nth most recent command
exit/quit          # Exit the shell

//Run any regular unix command
ls -l              # List files in the current directory
cd /path/to/dir    # Change directory
mkdir mydir        # Create a directory
rm myfile.txt      # Remove a file
```
