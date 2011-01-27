#!/usr/bin/perl
$num = 1;
while($num < 500) {
open(FILE, "$num.mob") || print "No file $num.mob\r\n";
@all = <FILE>;
close(FILE);

foreach $member (@all) {
$member =~ s/Class: [0-9]/Class: 0/g;
push(@new, $member);
}

open(NEW, ">$num.mob");
foreach $line (@new) {
print NEW "$line";
}
close(NEW);
print "Fixing $num.mob\r\n";
$num++;  

}
