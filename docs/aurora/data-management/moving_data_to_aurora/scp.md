

## Data Transfer

Use scp and SFTP to transfer data to/from Aurora. 

### Transferring files from Aurora (Flare) to Sunspot (Gila)
From an Aurora login-node, you can transfer files to Sunspot's `Gila` filesystem using the scp command. But first, you need to create SSH keys on Aurora and copy the public key (*.pub) to the ~/.ssh/authorized_keys file on Sunspot.
1. Create SSH keys on the laptop/desktop/remote machine. See "Creating SSH Keys" section on [this page](https://help.cels.anl.gov/docs/linux/ssh/).
2. Copy the public key (*.pub) from ~/.ssh folder on Aurora to ~/.ssh/authorized_keys file on Sunspot (sunspot.alcf.anl.gov)
3. Run the scp command on Aurora to transfer files to Sunspot
```
haritha@aurora-uan-0010:~> scp test_file haritha@sunspot.alcf.anl.gov://lus/gila/projects/Aurora_deployment/haritha
...
haritha@uan-0001:/gila/Aurora_deployment/haritha> cat test_file
this is a test file

```

### Transferring files to Aurora (Flare)

With the bastion pass-through nodes currently used to access both Sunspot and Aurora, users will find it helpful to modify their `.ssh/config` files on Aurora appropriately to facilitate transfers to Aurora from other ALCF systems. These changes are similar to what Sunspot users may have already implemented. From an Aurora login-node, this readily enables one to transfer files from Sunspot's `gila` filesystem or one of the production filesystems at ALCF (`home` and `eagle`) mounted on an ALCF system's login node. With the use of `ProxyJump` below, entering the MobilePass+ or Cryptocard passcode twice will be needed (once for bastion and once for the other resource).  A simple example shows the `.ssh/config` entries for Polaris and the `scp` command for transferring from Polaris:

```
$ cat .ssh/config
knight@aurora-uan-0009:~> cat .ssh/config
Host bastion.alcf.anl.gov
    User knight

Host polaris.alcf.anl.gov
    ProxyJump bastion.alcf.anl.gov
    DynamicForward 3142
    user knight
```

```
knight@aurora-uan-0009:~> scp knight@polaris.alcf.anl.gov:/eagle/catalyst/proj-shared/knight/test.txt ./
---------------------------------------------------------------------------
                            Notice to Users
...
[Password:
---------------------------------------------------------------------------
                            Notice to Users
... 
[Password:
knight@aurora-uan-0009:~> cat test.txt 
from_polaris eagle
```


