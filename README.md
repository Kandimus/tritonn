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
>--force-run | -f

Ignore cold-restart

>--terminal | -t

Out logs end events to terminal

>--log *hex*

Set log mask as *hex*

>--simulate | -s

Simulate IO modules

>--no-dump | -D

Don't load dump files

>--config <file>

Force load config file <file>

>--no-set-IP | -I

Disable set IP addressed from config file

## run triton on PC (linux) ##
> ./tritonn -fstDI

## 3thparty libs ##
    sudo apt install libncurses5-dev
