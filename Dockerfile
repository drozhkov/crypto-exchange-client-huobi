FROM alpine AS build

RUN apk update
RUN apk upgrade
RUN apk add gcc
RUN apk add g++
RUN apk add boost-dev
RUN apk add openssl-dev
RUN apk add cmake
RUN apk add make

WORKDIR /build-root

COPY . .

WORKDIR /build-root/.build
RUN cmake -DCMAKE_BUILD_TYPE:STRING="Release" -DCMAKE_INSTALL_PREFIX:PATH="./" ..
RUN cmake --build .
RUN cmake --install .

FROM alpine

RUN apk update
RUN apk upgrade
RUN apk add boost
RUN apk add openssl

WORKDIR /client-bin
COPY --from=build /build-root/.build/bin/ .

ENTRYPOINT ["/client-bin/crypto-exchange-client-huobi-demo"]

# CMD ["sh"]
