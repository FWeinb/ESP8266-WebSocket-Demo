BIN = $(PWD)/node_modules/.bin/


clean: cleanWeb cleanBuild
uploadAll: upload uploadfs

build:
	platformio run

cleanBuild:
	platformio run --target=clean

upload:
	platformio run --target=upload

uploadfs: webapp
	platformio run --target=uploadfs

cleanWeb:
	rm -f data/*

webapp: cleanWeb
	$(BIN)webpack
	gzip -f data/*
