import logging
import logging.handlers
import copy

class WukongHandler(logging.handlers.MemoryHandler):
    def shouldFlush(self, record):
        return False
        
    def retrieve(self):
        buffers = [logrecord.getMessage() for logrecord in self.buffer]
        self.flush()
        return buffers
