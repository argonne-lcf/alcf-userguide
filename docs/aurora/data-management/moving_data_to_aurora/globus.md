# Transferring files through Globus

## During Acceptance Testing period

We have set up a temporary Globus endpoint for Flare that you can use to transfer data out. The endpoint is called "alcf#dtn_flare_at" and is set up for read-only access. The endpoint will be available through the acceptance testing (AT) period and will be shutdown after AT concludes.

## Before Acceptance Testing

Currently, only Globus Personal is supported on Aurora. Perform the following steps to transfer data to/from the Aurora login nodes.

1. On a fresh connection to the login nodes, ensuring no proxies are being set
   (which may require commenting the proxy settings in the `~/.bashrc` or
   `~/.bash_profile` files), execute:

    ```bash
    /soft/tools/proxychains/bin/proxychains4 -f /soft/tools/proxychains/etc/proxychains.conf /soft/tools/globusconnect/globusconnect -setup --no-gui
    ```

1. Paste the link provided by the above command in a browser and follow the instructions to setup a personal endpoint

    - When requested, input your ALCF username and one-time password from your CRYPTOCard/MobilePASS+ token
    - Select the Allow button
    - Enter the authentication code generated back into the terminal
    - Enter a name for the endpoint (e.g., `aurora_login_uan11`)

1. On the same terminal, execute:

    ```bash
    /soft/tools/proxychains/bin/proxychains4 -f /soft/tools/proxychains/etc/proxychains.conf /soft/tools/globusconnect/globusconnect -start &
    ```

    - By default, the command only gives access to your home directory
    - You can add `-restrict-paths /lus/flare/projects/YOURPROJECT` to access your project directory

1. Open the [Globus web app](https://app.globus.org/file-manager?destination_id=05d2c76a-e867-4f67-aa57-76edeb0beda0) and search for the endpoint name defined above. You will now see your home directory (and project directory, if requested) on Aurora and can initiate transfers with other endpoints (e.g., the Eagle file system on Polaris at `alcf#dtn_eagle`)
