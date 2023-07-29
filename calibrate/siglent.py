import usbtmc


def demo():
    """
    example how to open Siglent SDS1102CML SCPI interface
    :return:
    """
    instr = usbtmc.Instrument(0xf4ec, 0xee3a)
    print(instr.ask("*IDN?"))
