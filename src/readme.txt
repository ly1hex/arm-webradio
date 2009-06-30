WebRadio v0.02 (c) Andreas Watterott (www.watterott.net)
========================================================
  Compiled with Rowley's CrossWorks for ARM 2.0
                CPU Support Pkg Luminary_LM3S 1.8


Third party software
--------------------
  Luminary Micro DriverLib 4694
    http://www.luminarymicro.com
    Modifications:
      lmi/driverlib/cpu.c: 46: PRIMASK -> primask
      lmi/driverlib/cpu.c:107: PRIMASK -> primask
      lmi/driverlib/interrupt.c:108: vtable -> .vectors_ram

  ChaN's FatFs 0.07c
    http://elm-chan.org/fsw/ff/00index_e.html


History
-------
         2009  v0.02  VS1053b FLAC decoder plugin
                      Auto detection of VS type (VS1053b, VS1033c, VS1033d)
                      Automatic standby
                      LFN support
                      Modify settings via Webinterface
                      Modify settings in the radio menu
                      DNS client

  Mar 16 2009  v0.01  First release


License
-------
  See license.txt
