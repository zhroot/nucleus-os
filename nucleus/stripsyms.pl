
while (<STDIN>) {
	print "$1 $2\n" if (m/0x([0-9a-fA-F]{8}) (.*$)/);
}
