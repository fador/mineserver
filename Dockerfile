FROM debian:jessie

MAINTAINER Marko Viitanen <fador@iki.fi>

    ENV PACKAGES libssl1.0.0 libevent-2.0 zlib1g systemd-sysv sysv-rc debconf passwd dpkg gnupg libnoise0
    ENV TEMP_PACKAGES build-essential cmake git  libssl-dev libevent-dev zlib1g-dev libnoise-dev ca-certificates 
    ENV DEBIAN_FRONTEND=noninteractive
    RUN apt-get update \
    && apt-get install -y --fix-missing --no-install-recommends --no-install-suggests $PACKAGES $TEMP_PACKAGES \
    && git clone --depth=1 git://github.com/fador/mineserver.git; \
        cd mineserver; \
        cmake .; \        
        make all;\
        make install; \
        make clean; \
    AUTO_PACKAGES=`apt-mark showauto`; \
    apt-get remove --purge --force-yes -y $TEMP_PACKAGES $AUTO_PACKAGES; \
        apt-get clean autoclean; \
        apt-get autoremove -y; \
        rm -rf /var/lib/{apt,dpkg,cache,log}/ ;\
        mkdir /var/mineserver; \
        cp -rf files/* /var/mineserver/; \
        cp -rf bin/* /var/mineserver/

EXPOSE 25565
ENV PATH /usr/local/bin:$PATH
VOLUME ["/var/mineserver"]

WORKDIR /var/mineserver
ENTRYPOINT ["mineserver"]
CMD ["/etc/mineserver/config.cfg","+system.path.home=\"/var/mineserver\"","+system.path.plugins=\"/usr/local/bin/plugins\""]