all: shell.rk

shell.rk: *.c *.h mem.asm
	x80cc --org=256 --lst --no-startup --target=bin -Os shell.c cmd_copymove.c cmd_new.c cmd_freespace.c cmd_delete.c cmd_sel.c interface.c fs.c mem.asm
	cscript /E:jscript make-rka.js
	../../m80noi/x64/Release/m80noi.exe shell.sym

.PHONY: clean
clean:
	del shell.rk shell.s
