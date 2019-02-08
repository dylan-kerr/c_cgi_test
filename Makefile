TARGET = bin/app
OBJS = src/app.o vendor/sqlite3.o
CC = gcc
CFLAGS = -g -Og -std=c11

# Declare file-specific options.
src/app.o: CFLAGS += -pedantic -Wall -Wextra -Werror
vendor/sqlite3.o: CPPFLAGS += -DSQLITE_THREADSAFE=0 -DSQLITE_OMIT_LOAD_EXTENSION

.PHONY: all clean deploy

all: $(TARGET)

$(TARGET): $(OBJS)
	$(LINK.o) $^ $(LOADLIBES) $(LDLIBS) -o $@

# Declare any inter-dependencies.
src/app.o: vendor/sqlite3.h

clean:
	$(RM) $(TARGET) $(OBJS)

deploy: all
	docker rm -f httpd || true
	docker run -dit --rm --name httpd -p 5000:80 \
		-v /vagrant/httpd.conf:/usr/local/apache2/conf/httpd.conf \
		-v /vagrant/static/:/usr/local/apache2/htdocs/ \
		-v /vagrant/bin/:/usr/local/apache2/cgi-bin/ \
		httpd
