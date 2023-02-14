JLinkGDBServer:  

        $(Q)JLinkGDBServer -select USB -device $(CHIP) \  

        -endian little -if SWD -speed 4000 -noir -LocalhostOnly  

  

debug:  

        $(Q)make  

        $(Q)echo target remote localhost\:2331 > gdb.gdb  

        $(Q)echo monitor reset >> gdb.gdb  

        $(Q)echo monitor halt >> gdb.gdb  

        $(Q)echo load >> gdb.gdb  

        $(Q)echo b main >> gdb.gdb  

        $(Q)echo - >> gdb.gdb  

        $(Q)echo c >> gdb.gdb  

        $(Q)-$(GDB) $(BUILD)/$(TARGET).elf --command=gdb.gdb  

        $(Q)$(RM) gdb.gdb  

debug_ozone:
        $(Q)ozone ./debug_ozone.jdebug
  

download:  

        $(Q)make  

        $(Q)echo "h" > jlink.jlink  

        $(Q)echo "loadfile" $(BUILD)/$(TARGET).hex >> jlink.jlink  

        $(Q)echo "r" >> jlink.jlink  

        $(Q)echo "qc" >> jlink.jlink  

        $(Q)$(JLINKEXE) -device $(CHIP) -Speed 4000 -IF SWD -CommanderScript jlink.jlink  

        $(Q)$(RM) jlink.jlink  

  

reset:  

        $(Q)echo "r" >> jlink.jlink  

        $(Q)echo "qc" >> jlink.jlink  

        $(Q)$(JLINKEXE) -device $(CHIP) -Speed 4000 -IF SWD -CommanderScript jlink.jlink  

        $(Q)$(RM) jlink.jlink  