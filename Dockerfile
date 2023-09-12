FROM ubuntu:23.04

SHELL ["/bin/bash", "-c"]

RUN apt-get update && DEBIAN_FRONTEND=noninteractive TZ=Etc/UTC apt-get -y install \
	clang-format \
    clang-tidy \
    cmake \
    doxygen \
    gdb \
    iproute2 \
    iptables \
    jc \
    lcov \
    libbpf-dev \
    libcriterion-dev \
    python3-breathe \
    python3-pip \
    python3-scapy \
    python3-sphinx \
    vim

#RUN pip install furo
#RUN pip install scapy

WORKDIR /srv/bpfilter
COPY . .
RUN chmod -R a-w .

WORKDIR /srv/build
RUN cmake ../bpfilter

RUN make

ENV PATH="${PATH}:/srv/build/src"

CMD ["python3", "/srv/bpfilter/tests/end_to_end/end_to_end.py"]
