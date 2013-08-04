from collections import OrderedDict

class KeyValueStore(OrderedDict):
  def __init__(self, *args, **kargs):
    self.size_limit = kargs.pop("size_limit", None)
    OrderedDict.__init__(self, *args, **kargs)
    self._check_size_limit()

  def __setitem__(self, key, value):
    OrderedDict.__setitem__(self, key, value)
    self._check_size_limit()

  def _check_size_limit(self):
    if self.size_limit is not None:
      while sys.getsizeof(self) > self.size_limit:
        self.popitem(last=False)
