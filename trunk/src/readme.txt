WebRadio v0.01 (c) Andreas Watterott (www.watterott.net)
========================================================
  Compiled with Rowley's CrossWorks for ARM 1.7 Build 19
                CPU Support Pkg Luminary_LM3S 1.8


Third party software
--------------------
  Luminary Micro DriverLib 4423
    http://www.luminarymicro.com
    Modifications:
      lmi/driverlib/cpu.c: 46: PRIMASK -> primask
      lmi/driverlib/cpu.c:107: PRIMASK -> primask
      lmi/driverlib/interrupt.c:108: vtable -> .vectors_ram

  ChaN's FatFs 0.07
    http://elm-chan.org/fsw/ff/00index_e.html


History
-------
         2009  v0.02  Automatic standby
                      LFN support
                      Change settings in the Menu

  Mar 16 2009  v0.01  First release


License
-------
  See license.txt
