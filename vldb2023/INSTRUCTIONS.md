# Instructions

## Compile

```sh
gcc query.c -o query
```

## Run query

Check the [instructions for running a single query](RUNNING.md)

## Deploy jupyter notebook

Install and run jupyter-lab:
```sh
pip install jupyterlab
jupyter lab
```
The [notebook](demo.ipynb) will install the rest of the requirements

### IP address

Set the ip address of the fpga device on the ipynb file.

### .ssh/config

Add the following configuration to your .ssh/config file:
```sshconfig
Host 10.210.1.*
    User root
    HostKeyAlgorithms +ssh-rsa
    PubkeyAcceptedKeyTypes +ssh-rsa
```
