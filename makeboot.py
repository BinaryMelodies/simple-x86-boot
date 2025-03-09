#! /usr/bin/python3

import sys

def main():
	if len(sys.argv) <= 1:
		print(f"Usage: {sys.argv[0]} <image file name>")
		exit()
	with open(sys.argv[1], 'r+b') as file:
		file.seek(0x1FE)
		file.write(b'\x55\xAA')

if __name__ == '__main__':
	main()

