all: slc slpm

slc: mkdirs ast semantic codegen
	cd temp && bison -d ../parser/parser.y -o parser.tab.c
	cd temp && flex ../lexer/lexer.l
	cd temp && g++ -I. -std=c++17 -o ../bin/slc \
		parser.tab.c lex.yy.c \
		../ast/ast.cpp \
		../semantic/semantic.cpp \
		../codegen/codegen.cpp

slpm: mkdirs
	cd slpm && make
	cp slpm/slpm bin/

test: slc
	@echo "Running tests..."
	@./bin/slc tests/basic_test.sl /tmp/basic && /tmp/basic; echo "basic_test: $$?"
	@./bin/slc tests/expressions_test.sl /tmp/expressions && /tmp/expressions; echo "expressions_test: $$?"
	@./bin/slc tests/control_flow_test.sl /tmp/control_flow && /tmp/control_flow; echo "control_flow_test: $$?"
	@./bin/slc tests/functions_test.sl /tmp/functions && /tmp/functions; echo "functions_test: $$?"
	@./bin/slc tests/advanced_test.sl /tmp/advanced && /tmp/advanced; echo "advanced_test: $$?"
	@echo "Testing library creation..."
	@./bin/slc tests/library_test.sl -shared /tmp/mylib.so && echo "shared_lib: created" && rm /tmp/mylib.so
	@./bin/slc tests/library_test.sl -static /tmp/mylib.a && echo "static_lib: created" && rm /tmp/mylib.a

install: all
	@echo "Installing SL toolchain to /usr/local/bin/"
	cp bin/slc /usr/local/bin/
	cp bin/slpm /usr/local/bin/
	@echo "Installation complete!"
	@echo "Run 'slc' and 'slpm' from anywhere in your system"

uninstall:
	@echo "Uninstalling SL toolchain from /usr/local/bin/"
	rm -f /usr/local/bin/slc
	rm -f /usr/local/bin/slpm
	@echo "Uninstallation complete!"

clean:
	rm -rf bin temp /tmp/basic* /tmp/expressions* /tmp/control_flow* /tmp/functions* /tmp/advanced*
	cd slpm && make clean

ast: mkdirs
	@echo "AST structures ready"

semantic: mkdirs
	@echo "Semantic analyzer ready"

codegen: mkdirs
	@echo "Code generator ready"

mkdirs:
	@mkdir -p bin temp