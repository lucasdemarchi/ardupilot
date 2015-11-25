"""
Helper module for boards definitions.
"""

import re

attr_re = re.compile(r'^[A-Z_]+$')

def obj_to_dict(obj):
    return {k: getattr(obj, k) for k in dir(obj) if attr_re.match(k)}

class KeyValue(object):
    """
    Key-value object that can be easily extended by calling the object with
    keyword arguments.
    Example::

       kw = KeyValue(FOO='foo', BAR='bar') # kw has the keys FOO and BAR
       kw(BAZ='baz') # the key BAZ was added
       kw(BAR='overwrite') # key BAR has been overwritten
       kw(ABC='xyz', BAZ='baz!!!') # ABC added and BAZ overwritten
    """
    def __init__(self, **kw):
        self(**kw)

    def compile(self):
        """
        Return a list of "<KEY>=<VALUE>" strings.
        """
        return ['%s=%s' % item for item in obj_to_dict(self).items()]

    def __call__(self, **kw):
        for k, v in kw.items():
            if attr_re.match(k):
                setattr(self, k, v)

class List(list):
    def __call__(self, *k):
        self.extend(k)

class Board(object):
    """
    Class for a board definition for the waf build system. The board()
    decorator function can be used as a helper to easily extend this class.
    """
    board_classes = {}

    @staticmethod
    def get(name):
        """
        Instantiate a registered board class.
        """
        if name in Board.board_classes:
            return Board.board_classes[name]()

        raise Exception('board %s not found' % name)

    @staticmethod
    def names():
        """
        Return a sorted list of the registered boards names.
        """
        return sorted(list(Board.board_classes.keys()))

    @staticmethod
    def register(name, cls):
        """
        Register a board class with a name.
        """
        Board.board_classes[name] = cls

    def __init__(self, name):
        self.name = name

        self.DEFINES = KeyValue()
        self.LIB = List()
        self.AP_LIBRARIES = List()
        self.CXXFLAGS = List()

    def compile(self):
        """
        Generate a dictionary to be used in waf env.
        """
        d = obj_to_dict(self)
        for k, v in d.items():
            if isinstance(v, KeyValue):
                d[k] = v.compile()

        return d

def inherit(base=Board):
    """
    Create a decorator that receives a constructor function and creates a class
    representing a board. The class created inherits from the class pointed by
    board (default is Board).

    The following code::

       class MyBoard(Board):
           def __init__(self):
               super(MyBoard, self).__init__('my_board')

               self.DEFINES(
                   DEBUG='1',
               )

       Board.register('my_board', MyBoard)

    Is equivalent to::

       @board.inherit()
       def my_board(board):
           board.DEFINES(
               DEBUG='1',
           )
    """
    def decorator(constructor):
        name = constructor.__name__

        def __init__(self, name=name):
            base.__init__(self, name)
            constructor(self)

        cls = type(name, (base,), dict(__init__=__init__))
        Board.register(name, cls)

        return cls

    return decorator
