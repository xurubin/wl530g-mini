all: root.tar.lzfx

root.tar.lzfx: 
	@rm -f $@
	@tar cO --mode=755 -C root/ . | ./lzfx.exe /dev/stdin $@ c 2>/dev/null
	@echo  "========== Content =============="
	@./lzfx.exe $@ /dev/stdout d  2>/dev/null | tar tf -
	@ls -l $@

.PHONY: all root.tar.lzfx