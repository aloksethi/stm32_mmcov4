# stm32_mmcov4
code for using nucleo-f429zi board with matlab.
will be porting the old code to use the ethernet interface instead of the serial

This version of code was build using STM32CubeIDE release v1.7.

This should compile ok with the 1.9 version too but not migrating the code to the new libraries yet.
also checked with version 1.10.1 of STM32CubeIDE. haven't migrated the code n probably will never do. the downside is no fixes to the middleware.
also the inability to modify the .ioc file.

anyway, this project is going to maintainance only as the controller for the panel will be drastically different that it will be handled in its own project not in a separate branch as i was planning earlier.
