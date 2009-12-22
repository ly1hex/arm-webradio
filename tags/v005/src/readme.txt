WebRadio v0.05 (c) Andreas Watterott (www.watterott.net)
========================================================
  Compiled with Rowley's CrossWorks for ARM 2.04
                         CPU Support Pkg. Stellaris 1.1


Third party software
--------------------
  Luminary Micro StellarisWare Driver Library 5228
    http://www.luminarymicro.com
    Modifications:
      lmi/driverlib/cpu.c: 46: PRIMASK -> primask
      lmi/driverlib/cpu.c:107: PRIMASK -> primask
      lmi/driverlib/interrupt.c:108: vtable -> .vectors_ram

  ChaN's FatFs 0.07e
    http://elm-chan.org/fsw/ff/00index_e.html


License
-------
  See license.txt


History
-------
  Dec 22 2009  v0.05  IR commands changeable
                      Different Alarm actions

  Nov 29 2009  v0.04  Changes in VS interface
                      LCD L2F50 support

  Nov 18 2009  v0.03  Switched to CrossWorks CPU Support Pkg. Stellaris 1.1
                      Bootloader added
                      New buffer system
                      F-RAM support

  Aug 19 2009  v0.02  VS1053b FLAC decoder plugin
                      Auto detection of VS type (VS1053b, VS1033c, VS1033d)
                      Automatic standby
                      LFN support
                      Modify settings via Webinterface
                      Modify settings in the radio menu
                      DNS client

  Mar 16 2009  v0.01  First release
