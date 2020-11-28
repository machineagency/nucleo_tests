# Nucleo Bringup

## Download the Toolchain
The old [gcc-arm-embedded](https://launchpad.net/gcc-arm-embedded/) toolchain mentions that new versions of the toolchain will no longer be released on Launchpad, so you'll have to download them directly from Arm.

First download a compressed fle of the latest release from the [Arm Developer Downloads Page](https://developer.arm.com/tools-and-software/open-source-software/developer-tools/gnu-toolchain/gnu-rm/downloads).

Inflate it with:
```
tar -xjvf gcc-arm-none-eabi-7-20xx-qx-update-linux.tar.bz2
```
where the above file year and quarterly release value are updated.

Put the resulting folder somewhere in your system folder directory structure.
I made a folder called *Applications*, who's full path is */home/poofjunior/Applications*.

Add the *bin* subdirectory to your path permanantly by adding the line below to your *.bashrc* file in your home directory.
```
export PATH=$PATH:/home/poofjunior/Applications/gcc-arm-none-eabi-7-20xx-qx-update/bin/
```
Note that *poofjunior/Applications* is replaced with your actual path and the year and final folder name matches the one you downloaded.

At this point, you'll need to run ```source ~/.bashrc``` or open a new terminal before you can invoke the tools by name.


## Setting UDEV rules
By default, connecting to most external devices requires root priviledges.
Udev rules are a Linux construct that map specific users permissions to a particular device, and they even let your device appear with a consistent name.
We'll want our Black Magic Probe (which actually has two ports) to show up as two devices using a Udev rule.

Install Udev Rules by copying [99-blackmagic.rules](./99-blackmagic.rules) to your /etc/udev/rules.d folder.
Reload the rules with:
```
sudo udevadm control --reaload-rules
```
Then the two ports should show up in your /dev directory as **ttyBmpGdb** and **ttyBmpTarg**.

## References
* [How to Install Arm Toolchain on Ubuntu](https://unix.stackexchange.com/questions/453032/how-to-install-a-functional-arm-cross-gcc-toolchain-on-ubuntu-18-04-bionic-beav)
* [STM32F76xxx Datasheet](https://www.st.com/resource/en/reference_manual/dm00224583-stm32f76xxx-and-stm32f77xxx-advanced-armbased-32bit-mcus-stmicroelectronics.pdf)
* [Nucleo-F767ZI Dev Board Datasheet](https://www.st.com/resource/en/user_manual/dm00244518-stm32-nucleo144-boards-mb1137-stmicroelectronics.pdf)
* [ARM Semihosting for print statements](https://bgamari.github.io/posts/2014-10-31-semihosting.html)
