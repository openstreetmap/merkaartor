#! /usr/bin/perl -w

use utf8;
use encoding "utf8";

my $desc;
my $all;

my %lang;

while(my $line = <>)
{
  chomp $line;
  if($line =~ /^ *<description +locale="([A-Za-z_]+)" *>(.*?)<\/description> *$/)
  {
    my $val = $2;
    my $l = $1;
    $val =~ s/&gt;/>/g;
    $val =~ s/&lt;/</g;
    $desc{$l} = $val;
  }
  elsif($line =~ /description/)
  {
    die "Can't handle line $line";
  }
  elsif(%desc)
  {
    my $en = $desc{"en"};
    die "No english string found in previous block: $line" if(!$en);
    delete $desc{"en"};
    foreach my $l (keys %desc)
    {
      ++$lang{$l};
      if(exists($all{$en}{$l}) && $all{$en}{$l} ne $desc{$l})
      {
        die "String mismatch for $en: $all{$en}{$l} != $desc{$l}";
      }
      $all{$en}{$l} = $desc{$l};
    }
    %desc = ();
  }
}

foreach my $la (keys %lang, "en")
{
  my $filename = $la eq "en" ? "templates.pot" : "templates_$la.po";
  die "Could not open outfile for $la\n" if !open FILE,">:utf8",$filename;
  print FILE "#, fuzzy\n"
  . "msgid \"\"\n"
  . "msgstr \"\"\n"
  . "\"Project-Id-Version: PACKAGE VERSION\\n\"\n"
  . "\"Report-Msgid-Bugs-To: \\n\"\n"
  . "\"POT-Creation-Date: 2009-01-01 12:00+0200\\n\"\n"
  . "\"PO-Revision-Date: YEAR-MO-DA HO:MI+ZONE\\n\"\n"
  . "\"Last-Translator: FULL NAME <EMAIL\@ADDRESS>\\n\"\n"
  . "\"Language-Team: LANGUAGE <LL\@li.org>\\n\"\n"
  . "\"MIME-Version: 1.0\\n\"\n"
  . "\"Content-Type: text/plain; charset=UTF-8\\n\"\n"
  . "\"Content-Transfer-Encoding: 8bit\\n\"\n"
  . "\"X-Generator: merkaartor templates\\n\"\n\n";

  my $num = 1;
  foreach my $en (keys %all)
  {
    my $str = ($la ne "en" && exists($all{$en}{$la})) ? $all{$en}{$la} : "";
    print FILE "#: #$num\nmsgid \"$en\"\nmsgstr \"$str\"\n\n";
    ++$num;
  }
  close FILE;
}
