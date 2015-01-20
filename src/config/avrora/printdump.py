import intelhex
ih = intelhex.IntelHex("flashdump.ihex")
start_address = 2 * 0x2680
number_of_bytes = 80
bytes = [ih[start_address+i] for i in range(number_of_bytes)]
for i in range(0,number_of_bytes,2):
	print '{:02x}'.format(bytes[i]), '{:02x}'.format(bytes[i+1])