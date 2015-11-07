#!/opt/local/bin/perl
# This script will convert Panasonic codes from the long_ir_rec script by AnalysIR for Arduino into whats needed by the IRremote library
# H.Vestergaard 2015
use strict;

my @bit_frame;
my @byte_frame = (0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0);;

my $dummy = `cat ir_off_raw`;
my $n = 1;
while ( $dummy =~ /(-?\d+)/g ) {
	my $bitcode = $1;
	if ( (($bitcode > -500) and ($bitcode < -300)) or (($bitcode > -1400) and ($bitcode < -1200)) ) {
		if ($bitcode <= -1200) {
			$bit_frame[$n] = 1; 
		} else {
			$bit_frame[$n] = 0;
		}
		$n++;
	}
}
my $crc_byte = 0;
for (my $i=1; $i <= 27; $i++) {
	if ($bit_frame[$i*8-7] == 1) { $byte_frame[$i] = 1 };
	if ($bit_frame[$i*8-6] == 1) { $byte_frame[$i] = $byte_frame[$i] | 2 };
	if ($bit_frame[$i*8-5] == 1) { $byte_frame[$i] = $byte_frame[$i] | 4 };
	if ($bit_frame[$i*8-4] == 1) { $byte_frame[$i] = $byte_frame[$i] | 8 };
	if ($bit_frame[$i*8-3] == 1) { $byte_frame[$i] = $byte_frame[$i] | 16 };
	if ($bit_frame[$i*8-2] == 1) { $byte_frame[$i] = $byte_frame[$i] | 32 };
	if ($bit_frame[$i*8-1] == 1) { $byte_frame[$i] = $byte_frame[$i] | 64 };
	if ($bit_frame[$i*8] == 1) { $byte_frame[$i] = $byte_frame[$i] | 128 };
	if ( ($i >= 9) && ($i <= 26) ) { $crc_byte = $crc_byte + $byte_frame[$i] }; 		# To check correct crc at byte 27
	printf '%#x ',  $byte_frame[$i];
}

printf "\n Check the byte representation of the crc code is equal to last byte: %#x", $crc_byte;
print "\n";
