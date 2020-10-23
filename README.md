# tritonn

## Setup git

    # ssh-keygen -t rsa -C "user.name@mail.domain"
    # gedit ~/.ssh/id_rsa

Copy text to clipboard, open github.com, click on **settings** -> **SSH and GPG keys**. Add new SSH-key from clipboard.

    # git config --global user.name "YourFullName"
    # git config --global user.email myemail@mail.ru

## Get ##
    # cd ~/projects/tritonn
    # git fetch
    # git checkout master
    # git pull
    # git submodule update

## Build ##
    # cd ~/projects/tritonn
    # ./tritonn_build

## Run ##
    # cd ~/tritonn
    # ./tritonn

## Command line ##
>--force-run

Ignore cold-restart

>--terminal

Out logs end events to terminal

>--force-conf *filename*

Open config ./conf/*filename*

>--log *hex*

Set log mask as *hex*

>--simulate

Simulate IO modules
