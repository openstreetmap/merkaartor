#! /usr/bin/perl -w

use utf8;
use encoding "utf8";
use File::Copy;

my %files = map {$_ => undef} ("merkaartor", "templates");
my $templ = "(".join("|",keys %files).")";
foreach my $f (@ARGV)
{
  if($f =~ /\*/) { printf "Skipping $f\n"; }
  elsif($f =~ /${templ}_(.*)\.po$/) { $files{$1}{$2} = $f; }
  elsif($f =~ /${templ}\.pot$/) { $files{$1}{pot} = $f; }
  elsif($f =~ /^${templ}-/) { warn "Skip $f"; }
  else { die "unknown file name $f."; }
}
mkdir "upl";
foreach my $t (keys %files)
{
  mkdir "upl/$t";
}

my $temp = "templates";
copy($files{$temp}{"pot"}, "upl/$temp/$temp.pot");
foreach my $t (keys %{$files{$temp}})
{
  next if $t eq "pot";
  copy($files{$temp}{$t}, "upl/$temp/$t.po");
}

$temp = "merkaartor";
copy($files{$temp}{"pot"}, "upl/$temp/$temp.pot");
foreach my $t (keys %{$files{$temp}})
{
  next if $t eq "pot";
  copy($files{$temp}{$t}, "upl/$temp/$t.po") if -f "${temp}_$t.ts";
}

chdir "upl";
my @t=gmtime();
my $date=sprintf("%04d-%02d-%02d_%02d_%02d", 1900+$t[5],$t[4]+1,$t[3],$t[2],$t[1]);

system "tar -czf ../${date}_launchpad_upload.tgz *";
chdir "..";

foreach my $t (keys %files)
{
  unlink glob("upl/$t/*");
  rmdir "upl/$t";
}
rmdir "upl"