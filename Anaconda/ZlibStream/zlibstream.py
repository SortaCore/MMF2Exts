# Anaconda copyright (c) 2011 Mathias Kaerlev
# See LICENSE for details.

# zlibstream.mfx
# ZlibStream object - Darkwire Software (http://dark-wire.com/)
# Copyright © 2010-2012 Darkwire Software:
# Use of this software is subject to the following conditions:
# • This copyright and conditions must be included in any distributions.
# • This file may not be modified any person or group that are not under the employ of Darkwire Software.
# Failure to comply with these conditions may result in legal action.
# Also, Monty Python and the Holy Grail is cool. Except for one part nearish the start. )

# Multi-threaded compression/decompression stream extension.

# Ported to Python by SortaCore
# Original C++ code created by SortaCore
# GZIP code: http://www.gzip.org/zlib/manual.html
# ZLIB code: http://www.zlib.org/
import traceback
from mmfparser.player.extensions.common import UserExtension, HiddenObject
from mmfparser.player.event.actions.common import Action
from mmfparser.player.event.conditions.common import Condition
from mmfparser.player.event.expressions.common import Expression

from sys import *   # Import sys for uncompressed file writing/reading
import zlib
import gzip         # Import gzip for compressed file writing/reading
import thread       # Import thread for multithreading
import time         # Import time for thread sleeping

# Please note: Using GenerateEvent in place of PushEvent is not a good idea, being that
# access to rdPtr is blocked by instance.objectPlayer.threadsafe.acquire(). Example code:
#   instance.objectPlayer.threadsafe.acquire()
#   GenerateEvent(0)
#   instance.objectPlayer.threadsafe.release()
# If the user retrieves expressions with GenerateEvent, the ThreadSafe_End is not run
# until after they retrieve the variables. So of course the while loop in ThreadSafe_Start
# runs without stopping.
# PushEvent is slower, being called on next MMF loop, which means the expressions are accessed
# after: No "infinite-while-loop" problem.
# So here we have the caller type:


#
# Non-MMF-called Functions
#

# Retrieves a file's size
def file_size(filename):
    TempFile = open(filename, "rb", 0)     # Open file
    if TempFile == None: return 0          # If handle has a problem, return 0, eg no file
    TempFile.seek(0, 2)                    # 2 == SEEK_END - If not, goto end of file
    size = TempFile.tell()                 # Get position (Must be stored so fclose can be run)
    TempFile.close()                       # Close file
    return size                            # Return

# Check files sizes
def FileSizeCheck(infilename, instance):
    if file_size(infilename) < 1:
        if instance.objectPlayer.threadsafe.locked(): # Threadsafe lock on
           print "FileSizeCheck() halted, locked() returned true"
        while instance.objectPlayer.threadsafe.locked(): # Threadsafe lock on
            time.sleep(0)
        print "FileSizeCheck() acquired lock"
        instance.objectPlayer.threadsafe.acquire()
        instance.objectPlayer.returnstring = "Input file size 0 or nonexistent."
        instance.objectPlayer.threadsafe.release() # Threadsafe lock off
        return True
    else:
        return False

# Check handles of both objects
def HandleCheck(regfile, gzfile, instance):
    if regfile == None:
        if instance.objectPlayer.threadsafe.locked():
            print "regfile == None inside HandleCheck() halted, locked() is true."
        while instance.objectPlayer.threadsafe.locked(): # Threadsafe lock on
            time.sleep(0)
        print "HandleCheck() acquired lock (regfile)"
        instance.objectPlayer.threadsafe.acquire()
        instance.objectPlayer.returnstring = "Input file malfunctioned."
        gzfile.close()
        instance.objectPlayer.threadsafe.release() # Threadsafe lock off
        return True
    elif gzfile == None:
        if instance.objectPlayer.threadsafe.locked(): # Threadsafe lock on
            print "gzfile == None inside HandleCheck() halted, locked() is true."
        while instance.objectPlayer.threadsafe.locked(): # Threadsafe lock on
            time.sleep(0)
        print "HandleCheck() acquired lock (gzfile)"
        instance.objectPlayer.threadsafe.acquire()
        instance.objectPlayer.returnstring = "Output file malfunctioned."
        regfile.close()
        instance.objectPlayer.threadsafe.release() # Threadsafe lock off
        return True
    else:
        return False

#
# Threads
#

# Compression Thread
def CompressionThread(infilename, outfilename, UseAppendI, instance, *event):
    WriteType = "wb"                 # User is using write (overwriting)
    if UseAppendI: WriteType = "ab"  # User is using append

    # Check input file size - no handles to close
    if FileSizeCheck(infilename, instance):
        if instance.objectPlayer.threadsafe.locked():
            print "Thread halted on FileSizeCheck(), locked() is true."
        while instance.objectPlayer.threadsafe.locked(): # Threadsafe lock on
            time.sleep(0)
        print "Thread passed FileSizeCheck(); locked() is now acquired."
        instance.objectPlayer.threadsafe.acquire()
        instance.objectPlayer.LastOutput = outfilename
        event.fire_handler('OnError')
        instance.objectPlayer.threadsafe.release() # Threadsafe lock off
        return True

    # Open handles to both files
    infile = open(infilename, "rb")                     # r = read, b = binary
    outfile = open(outfilename, WriteType)    # a = append (or w=write), b = binary, 9 = max compression

    # Check handles - HandleCheck() automatically closes handles if invalid
    if HandleCheck(infile, outfile, instance):
        if instance.objectPlayer.threadsafe.locked():
            print "Thread halted on HandleCheck(), locked() is true."
        while instance.objectPlayer.threadsafe.locked(): # Threadsafe lock on
            time.sleep(0)
        print "Thread passed HandleCheck(); locked() is now acquired."
        instance.objectPlayer.threadsafe.acquire()
        instance.objectPlayer.LastOutput = outfilename
        event.fire_handler('OnError')
        instance.objectPlayer.threadsafe.release() # Threadsafe lock off
        return True

    # Declare variables
    PreviousOutputSize = 0 # For calculation purposes
    num_read = 0
    total_read = 0
    if instance.objectPlayer.threadsafe.locked():
        print "Thread halted on buffersize retrieval, locked() is true."
    while instance.objectPlayer.threadsafe.locked(): # Threadsafe lock on
        time.sleep(0)
    print "Thread passed buffersize retrieval; locked() is now acquired."
    instance.objectPlayer.threadsafe.acquire()
    tempinbuffersize = instance.objectPlayer.buffersize
    instance.objectPlayer.threadsafe.release()     # Threadsafe lock off


    # If using Write, output size should not be included in the calculation later. Otherwise:
    if UseAppendI: PreviousOutputSize = file_size(outfilename)

    # This makes sure that the buffer is the right size - if too large, set buffer smaller
    if file_size(infilename) < tempinbuffersize:
        tempinbuffersize = file_size(infilename)

    print "Beginning to read from input..."
    # Iteration through the files
    inbuffer = infile.read(tempinbuffersize)
    c = zlib.compressobj(9) # compressor
    while inbuffer:
        total_read += len(inbuffer)
        outfile.write(c.compress(inbuffer))
        inbuffer = infile.read(tempinbuffersize)

    outfile.write(c.flush())
    
    print "Reading done, thread calculating variables"
    # Close thread
    infile.close()
    outfile.close()
    del inbuffer

    # Set variables after completion
    if instance.objectPlayer.threadsafe.locked():
        print "Thread halted on setting variables, locked() is true."
    while instance.objectPlayer.threadsafe.locked():
        time.sleep(0)
    print "Thread passed buffersize retrieval; locked() is now acquired."
    instance.objectPlayer.threadsafe.acquire()
    saveoutfilesize = file_size(outfilename)
    saveinfilesize = file_size(infilename)
    instance.objectPlayer.PercentageDifference = float(((file_size(outfilename)-PreviousOutputSize)*(1.0/file_size(infilename)))*100.0)

    instance.objectPlayer.returnstring = "Buffer used: %(a)d, total bytes read: %(b)d, total bytes written: %(c)d, compression rate: %(d).2f%%." % \
    {"a": tempinbuffersize, "b" : file_size(infilename), "c" : file_size(outfilename)-PreviousOutputSize, "d": instance.objectPlayer.PercentageDifference}
     
    instance.objectPlayer.LastOutput = outfilename
    event.fire_handler('OnSuccess') # if expression read while this is fired, will the expression halt?!
    instance.objectPlayer.threadsafe.release()
    print "Thread is done!"
    return 0


# Decompression Thread
def DecompressionThread(infilename, outfilename, UseAppendI, instance, event):
    WriteType = "wb"                      # User is using write (overwriting)
    if UseAppendI: WriteType = "ab"       # User is using append

    if FileSizeCheck(infilename, instance):
        while instance.objectPlayer.threadsafe.locked(): # Threadsafe lock on
            time.sleep(0)
        instance.objectPlayer.threadsafe.acquire()
        instance.objectPlayer.LastOutput = outfilename
        event.fire_handler('OnError')
        instance.objectPlayer.threadsafe.release() # Threadsafe lock off
        return True

    # Open handles to both files
    infile = file(infilename, "rb") # r = read, b = binary
    outfile = file(outfilename, WriteType)      # a = append or w = write, b = binary

    if HandleCheck(outfile, infile, instance):
        while instance.objectPlayer.threadsafe.locked(): # Threadsafe lock on
            time.sleep(0)
        instance.objectPlayer.threadsafe.acquire()
        instance.objectPlayer.LastOutput = outfilename
        event.fire_handler('OnError')
        instance.objectPlayer.threadsafe.release() # Threadsafe lock off
        return True

    # Declare variables
    PreviousOutputSize = 0 # For calculation purposes
    num_read = 0
    total_read = 0
    while instance.objectPlayer.threadsafe.locked():
        time.sleep(0)
    instance.objectPlayer.threadsafe.acquire()
    tempinbuffersize = instance.objectPlayer.buffersize
    instance.objectPlayer.threadsafe.release()

    # If using Write, output size should not be included in the calculation later. Otherwise:
    if UseAppendI: PreviousOutputSize = file_size(outfilename)

    # This makes sure that the buffer is the right size - if too large, set buffer smaller
    if file_size(infilename) < tempinbuffersize:
        tempinbuffersize = file_size(infilename)

    print "Thread is beginning to read input..."
    # Iteration through the files
    inbuffer = infile.read(tempinbuffersize)
    c = zlib.decompressobj()
    
    while inbuffer:
        total_read += len(inbuffer)
        outfile.write(c.decompress(inbuffer))
        inbuffer = infile.read(tempinbuffersize)

    outfile.write(c.flush())
    print "Reading done, thread calculating variables"
    # Close thread
    infile.close()
    outfile.close()
    del inbuffer

    # Set variables after completion
    while instance.objectPlayer.threadsafe.locked():
        time.sleep(0)
    instance.objectPlayer.threadsafe.acquire()
    instance.objectPlayer.PercentageDifference = float(((file_size(outfilename)-PreviousOutputSize)*(1.0/file_size(infilename)))*100.0)

    instance.objectPlayer.returnstring = "Buffer used: %(a)d, total bytes read: %(b)d, total bytes written: %(c)d, decompression rate: %(d).02f%." % \
    {"a": tempinbuffersize, "b" : file_size(infilename), "c" : file_size(outfilename)-PreviousOutputSize, "d": instance.objectPlayer.PercentageDifference}
    
    instance.objectPlayer.LastOutput = outfilename
    event.fire_handler('OnSuccess')
    instance.objectPlayer.threadsafe.release()
    print "Thread is done!"
    return 0

#
# Conditions - both are triggered
#

# ID = 0
class OnError(Condition):
    def created(self):
        self.add_handlers(on_error = self.on_error)
    
    def on_error(self, instance):
        self.generate(instance)

    def check(self, instance):
        return True

# ID = 1
class OnSuccess(Condition):
    def created(self):
        self.add_handlers(on_success = self.on_success)
    
    def on_success(self, instance):
        self.generate(instance)

    def check(self, instance):
        return True


#
# Expressions
#

# ID = 0
class DeprecatedLastReturnedValue(Expression):
    def get():
        raise DeprecationWarning("You are using an old \"LastReturnedValue\" expression. This is deprecated and should be removed.")
        return False

# ID = 1
class LastReturnedString(Expression):
    def get(self, instance):
        while instance.objectPlayer.threadsafe.locked():
            time.sleep(0)
        instance.objectPlayer.threadsafe.acquire()
        s = str(instance.objectPlayer.returnstring)
        instance.objectPlayer.threadsafe.release()
        return s

# ID = 2
class CurrentBufferSize(Expression):
    def get(self, instance):
        while instance.objectPlayer.threadsafe.locked():
            time.sleep(0)
        instance.objectPlayer.threadsafe.acquire()
        i = int(instance.objectPlayer.buffersize)
        instance.objectPlayer.threadsafe.release()
        return i

# ID = 3
class LastPercentageDifference(Expression):
    def get(self, instance):
        while instance.objectPlayer.threadsafe.locked():
            time.sleep(0)
        instance.objectPlayer.threadsafe.acquire()
        temp = float(instance.objectPlayer.PercentageDifference)
        instance.objectPlayer.threadsafe.release()
        # This casts to 2 decimal places
        sprintfdest = ""
        sprintf(sprintfdest, "%.2f", float(temp))
        temp2 = float(sprintfdest)
        return temp2

# ID = 4
class LastOutputFile(Expression):
    def get(self, instance):
        while instance.objectPlayer.threadsafe.locked():
            time.sleep(0)
        instance.objectPlayer.threadsafe.acquire()
        s = str(instance.objectPlayer.LastOutput)
        instance.objectPlayer.threadsafe.release()
        return s

#
# Actions
#

# ID = 0
class DeprecatedStartCompression(Action):
    """
    Deprecated action to start a compression (2 parameters instead of 3)
    """
    def execute(self, instance):
        raise DeprecationWarning("You are using an old \"Compress file\" action. This is deprecated and should be removed and recreated.")
        return False

# ID = 1
class DeprecatedStartDecompression(Action):
    """
    Deprecated action to start a decompression (2 parameters instead of 3)
    """
    def execute(self, instance):
        raise DeprecationWarning("You are using an old \"Decompress file\" action. This is deprecated and should be removed and recreated.")
        return False

# ID = 2
class SetBufferSize(Action):
    """
    Set the buffer size of threads that are created later.
    """
    def execute(self, instance):
        while instance.objectPlayer.threadsafe.locked():
            time.sleep(0)
        instance.objectPlayer.threadsafe.acquire()
        i = int(self.evaluate_expression(self.get_parameter(0)))
        if i > 0 and i < 16385:
            instance.objectPlayer.buffersize = i
            instance.objectPlayer.threadsafe.release()
        else:
            instance.objectPlayer.returnstring = "Buffer is an invalid size. Must be between 0 and 16385 (exclusive)."
            instance.objectPlayer.threadsafe.release()
            event.fire_handler('OnError')

# ID = 3
class StartCompression(Action):
    """
    Start compression thread

    Parameters:
    0: File to read
    1: File to write to
    2: Use write or append in file operations
    """
    def execute(self, instance):
        infile = str(self.evaluate_expression(self.get_parameter(0)))
        outfile = str(self.evaluate_expression(self.get_parameter(1)))
        appendto = bool(self.evaluate_expression(self.get_parameter(2)))
        print "infile is ", infile, "\noutfile is ", outfile, "\nappendto is ", appendto
        thread.start_new_thread(CompressionThread, (infile, outfile, appendto, instance, self))

# ID = 4
class StartDecompression(Action):
    """
    Start decompression thread

    Parameters:
    0: File to read
    1: File to write to
    2: Use write or append in file operations
    """
    def execute(self, instance):
        infile = str(self.evaluate_expression(self.get_parameter(0)))
        outfile = str(self.evaluate_expression(self.get_parameter(1)))
        appendto = bool(self.evaluate_expression(self.get_parameter(2)))
        thread.start_new_thread(DecompressionThread, (infile, outfile, appendto, instance, self))

#
# Finally, Anaconda schizzle
#
class DefaultObject(HiddenObject):
    threadsafe = thread.allocate_lock() # Ensure only one thread accesses rdPtr's variables
    PercentageDifference = float(0.0)   # Percentage difference input->output
    LastOutput = ""                     # Last finished file name
    returnstring = ""                   # String to return with statistics or errors
    buffersize = 8192                   # Default size of memory buffer
    def created(self):
        return
    
class ZlibStreamObject(UserExtension):
    objectPlayer = DefaultObject

    conditions = {
        0 : OnSuccess,
        1 : OnError,
    }

    actions = {
        0 : DeprecatedStartCompression,
        1 : DeprecatedStartDecompression,
        2 : SetBufferSize,
        3 : StartCompression,
        4 : StartDecompression,
    }

    expressions = {
        0 : DeprecatedLastReturnedValue,
        1 : LastReturnedString,
        2 : CurrentBufferSize,
        3 : LastPercentageDifference,
        4 : LastOutputFile,
    }

extension = ZlibStreamObject()

def get_extension():
    return extension
