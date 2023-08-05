#!/usr/bin/env python

import sys
import re


def demo_sds1102_cml():
    """
    example how to open Siglent SDS1102CML SCPI interface
    :return:
    """
    import usbtmc
    instr = usbtmc.Instrument(0xf4ec, 0xee3a)  # SDS1102CML
    print(instr.ask("*IDN?"))


def prn_hack(SCOPEID: str) -> None:
    import hashlib

    Model = 'SDS2000X+'

    # Note that 'AWG' should be used for the 'FG' option
    # If you have the 100 MHz model, then first upgrade it to 200 MHz, then 350 MHz and finally 500 MHz
    bwopt = (
        '25M', '40M', '50M', '60M', '70M', '100M', '150M', '200M', '250M', '300M', '350M', '500M', '750M', '1000M',
        'MAX',
        'AWG', 'WIFI', 'MSO', 'FLX', 'CFD', 'I2S', '1553', 'PWA', 'SENT', 'MANC')

    hashkey = '5zao9lyua01pp7hjzm3orcq90mds63z6zi5kv7vmv3ih981vlwn06txnjdtas3u2wa8msx61i12ueh14t7kqwsfskg032nhyuy1d9vv2wm925rd18kih9xhkyilobbgy'

    def gen(x):
        h = hashlib.md5((
                                hashkey +
                                (Model + '\n').ljust(32, '\x00') +
                                x.ljust(5, '\x00') +
                                2 * ((SCOPEID + '\n').ljust(32, '\x00')) +
                                '\x00' * 16).encode('ascii')
                        ).digest()
        key = ''
        for b in h:
            if (b <= 0x2F or b > 0x39) and (b <= 0x60 or b > 0x7A):
                m = b % 0x24
                b = m + (0x57 if m > 9 else 0x30)
            if b == 0x30: b = 0x32
            if b == 0x31: b = 0x33
            if b == 0x6c: b = 0x6d
            if b == 0x6f: b = 0x70
            key += chr(b)
        return key.upper()

    for opt in bwopt:
        n = 4
        line = gen(opt)
        print('{:5} {}'.format(opt, [line[i:i + n] for i in range(0, len(line), n)]))


def demo():
    """
    example how to open Siglent SDS1102CML SCPI interface
    :return:
    """
    import pyvisa
    rm = pyvisa.ResourceManager()
    # rm.list_resources()
    inst = rm.open_resource('USB0::0xF4EC::0x1011::SDS2PEEC7R1007::INSTR')
    idn = inst.query("*IDN?")
    s = inst.query("SCOPEID?")
    print(f"id={idn}, sid={s}")
    m = re.match("SCOPE_ID ([0-9a-z-]*)", s)
    if m:
        scopeid = re.sub("-", "", m.group(1))
        print(f"scopeid={scopeid}")
        prn_hack(scopeid)


def main() -> int:
    try:
        demo()
        return 0
    except Exception as e:
        print(f"Error: {e}", file=sys.stderr)
        return -1


if __name__ == '__main__':
    sys.exit(main())
