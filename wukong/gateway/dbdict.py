try:
   import cPickle as pickle
except:
   import pickle

import os, os.path
import UserDict
import sqlite3

def to_db_type(value):
    """
    If value's type is supported natively in SQLite, return value.
    Otherwise, return a pickled representation.
    """
    if value is None or isinstance(value, (int, long, float, basestring)):
        return value
    else:
        return buffer(pickle.dumps(value))

def from_db_type(value):
    """
    Converts a value from the database to a Python object.
    """
    if isinstance(value, buffer):
        return pickle.loads(str(value))
    else:
        return value

class DBDict(UserDict.DictMixin):
    """
    Shelf implementation using an SQLite3 database.
    """
    def __init__(self, filename):
        if not os.path.isfile(filename):
            self._database = sqlite3.connect(filename)
            self._database.execute("CREATE TABLE IF NOT EXISTS Shelf "
                               "(Key TEXT PRIMARY KEY NOT NULL, Value BLOB)")
        else:
            self._database = sqlite3.connect(filename)
        self._open = True

    def __del__(self):
        self.close()

    def __getitem__(self, key):
        row = self._database.execute("SELECT Value FROM Shelf WHERE Key=?",[key]).fetchone()
        if row:
            return from_db_type(row[0])
        else:
            raise KeyError(key)

    def __setitem__(self, key, value):
        self._database.execute("INSERT OR REPLACE INTO Shelf VALUES (?, ?)",[key, to_db_type(value)])
        self._database.commit()

    def __delitem__(self, key):
        if not self._database.execute("SELECT Key FROM Shelf WHERE Key=?",[key]).fetchone():
            raise KeyError(key)
        self._database.execute("DELETE FROM Shelf WHERE Key=?", [key])
        self._database.commit()

    def keys(self):
        """Return a list of keys in the shelf."""
        return [row[0] for row in self._database.execute("SELECT Key FROM Shelf")]

    def close(self):
        """Commit changes and close the file."""
        if self._database is not None:
            self._database.commit()
            self._database.close()
            self._database = None