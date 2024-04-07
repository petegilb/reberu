import unreal

class ReberuPython:
    def __init__(self):
        pass

    def test(self, input):
        print(f'{input.get_name()}')

    def reberu(self, input):
        self.test(input)


def main(input):
    reberu = ReberuPython()
    reberu.reberu(input)