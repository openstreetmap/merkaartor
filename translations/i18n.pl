#! /usr/bin/perl -w

use utf8;
use encoding "utf8";

my $mail = "Merkaartor <merkaartor\@openstreetmap.org>";
my %pokeys = (
  "Project-Id-Version" => "merkaartor_templates 1.0",
  "Report-Msgid-Bugs-To" => $mail,
  "POT-Creation-Date" => getdate(),
  "PO-Revision-Date" => getdate(),
  "Last-Translator" => $mail,
  "Language-Team" => $mail,
  "MIME-Version" => "1.0",
  "Content-Type" => "text/plain; charset=UTF-8",
  "Content-Transfer-Encoding" => "8bit",
  "X-Launchpad-Export-Date" => getdate(),
  "X-Generator" => "Merkaartor translation convert"
);

main();

sub getdate
{
  my @t=gmtime();
  return sprintf("%04d-%02d-%02d %02d:%02d+0000",
  1900+$t[5],$t[4]+1,$t[3],$t[2],$t[1]);
}

sub loadfiles($$@)
{
  my $desc;
  my $all;
  my ($lang,$keys,@files) = @_;
  foreach my $file (@files)
  {
    die "Could not open file $file." if(!open FILE,"<:utf8",$file);
    my $linenum = 0;
    if($file =~ /\.mat$/)
    {
      while(my $line = <FILE>)
      {
        ++$linenum;
        chomp $line;
        if($line =~ /^ *<description +locale="([A-Za-z_]+)" *>(.*?)<\/description> *$/)
        {
          my $val = maketxt($2);
          my $l = $1;
          $desc{$l} = $val;
          $desc{_file} = "$file:$linenum" if($l eq "en");
        }
        elsif($line =~ /description/)
        {
          die "Can't handle line $linenum in $file: $line";
        }
        elsif(%desc)
        {
          my $en = $desc{"en"};
          die "No english string found in previous block line $linenum in $file: $line" if(!$en);
          delete $desc{"en"};
          foreach my $l (keys %desc)
          {
            copystring(\%all, $en, $l, $desc{$l}, "line $linenum in $file", undef);
            ++$lang->{$l} if !($l =~ /^_/);
          }
          %desc = ();
        }
      }
    }
    elsif($file =~ /[-_](.._..)\.po$/ || $file =~ /^(?:.*\/)?(.._..)\.po$/ ||
    $file =~ /[-_](..)\.po$/ || $file =~ /^(?:.*\/)?(..)\.po$/)
    {
      my $l = $1;
      ++$lang->{$l};
      my %postate = (last => "", type => "");
      my $linenum = 0;
      while(<FILE>)
      {
        ++$linenum;
        my $fn = "$file:$linenum";
        chomp;
        if($_ =~ /^#/ || !$_)
        {
          checkpo(\%postate, \%all, $l, "line $linenum in $file");
          $postate{fuzzy} = ($_ =~ /fuzzy/);
        }
        elsif($_ =~ /^"(.*)"$/) {$postate{last} .= $1;}
        elsif($_ =~ /^(msg.+) "(.*)"$/)
        {
          my ($n, $d) = ($1, $2);
          checkpo(\%postate, \%all, $l, "line $linenum in $file");
          $postate{last} = $d;
          $postate{type} = $n;
          $postate{src} = $fn if($n eq "msgid");
        }
        else
        {
          die "Strange line $linenum in $file: $_.";
        }
      }
      checkpo(\%postate, \%all, $l, "line $linenum in $file");
    }
    elsif($file =~ /\.ts$/)
    {
      my $linenum = 0;
      my $ctx;
      my $loc;
      my $issource;
      my $istrans;
      my $source;
      my $trans;
      my $fuzzy;
      my $numerus;
      while(<FILE>)
      {
        ++$linenum;
        if(/<name>(.*)<\/name>/) {$ctx = $1; }
        elsif(/<location filename="(.*?)" line="(.*?)"\/>/) { $loc = "$1:$2"; }
        elsif(/context>/){$ctx = undef;}
        elsif(/message( numerus="yes")?>/)
        {
          if($source)
          {
          }
          $loc = $ctx = $issource = $istrans = $source = $trans = $fuzzy = undef;
          $numerus = $1;
        }
        elsif(/<\?xml/ || /<!DOCTYPE/ || /<\/TS>/){} # ignore
        # source
        elsif(/<source>(.*)<\/source>/){$source = $1;}
        elsif(/<source>(.*)/){$source = $1; $issource = 1;}
        elsif($issource && /(.*)<\/source>/){$source .= $1; $issource = undef;}
        elsif($issource){$source .= $_;}
        # translation
        elsif(/<translation( type="unfinished")?>(.*)<\/translation>/){$trans = $2;$fuzzy=$1;}
        elsif(/<translation( type="unfinished")?>(.*)/){$trans = $2; $istrans = 1;$fuzzy=$1;}
        elsif($istrans && /(.*)<\/translation>/){$trans .= $1; $istrans = undef;}
        elsif($istrans){$trans .= $_;}
# TODO handle numerus
        else
        {
          die "Strange line $linenum in $file: $_.";
        }

      }
    }
    else
    {
      die "File format not supported for file $file.";
    }
    close(FILE);
  }
  return %all;
}

sub copystring($$$$$$)
{
  my ($data, $en, $l, $str, $txt, $context) = @_;

  $en = "___${context}___$en" if $context;

  if(exists($data->{$en}{$l}) && $data->{$en}{$l} ne $str)
  {
    if($l =~ /^_/)
    {
      $data->{$en}{$l} .= ";$str";
    }
    else
    {
      my $f = $data->{$en}{_file} || $data->{$en}{_src} || "";
      warn "String mismatch for $en $txt: $data->{$en}{$l} != $str ($f)";
    }
  }
  else
  {
    $data->{$en}{$l} = $str;
  }
}

# TODO: Parse PO header strings
sub checkpo($$$$)
{
  my ($postate, $data, $l,$txt) = @_;
  if($postate->{type} eq "msgid") {$postate->{msgid} = $postate->{last};$postate->{msgid_pl}="";}
  elsif($postate->{type} eq "msgid_plural") {$postate->{msgid_1} = $postate->{last};}
  elsif($postate->{type} eq "msgstr[0]") {$postate->{msgstr} = $postate->{last};}
  elsif($postate->{type} eq "msgstr[1]") {$postate->{msgstr_1} = $postate->{last};}
  elsif($postate->{type} eq "msgstr[2]") {$postate->{msgstr_2} = $postate->{last};}
  elsif($postate->{type} eq "msgstr[3]") {$postate->{msgstr_3} = $postate->{last};}
  elsif($postate->{type} eq "msgctxt") {$postate->{context} = $postate->{last};}
  else
  {
    if($postate->{type} eq "msgstr") {$postate->{msgstr} = $postate->{last};}
    elsif($postate->{type}) { die "Strange type $postate->{type} found\n" }
    if((!$postate->{fuzzy}) && $postate->{msgstr} && $postate->{msgid})
    {
      copystring($data, $postate->{msgid}, "_src", $postate->{src},$txt,$postate->{context});
      copystring($data, $postate->{msgid}, $l, $postate->{msgstr},$txt,$postate->{context});
      if($postate->{msgstr_1})
      { copystring($data, $postate->{msgid}, "$l.1", $postate->{msgstr_1},$txt,$postate->{context}); }
      if($postate->{msgid_1})
      { copystring($data, $postate->{msgid}, "en.1", $postate->{msgid_1},$txt,$postate->{context}); }
    }
    delete $postate->{msgid};
    delete $postate->{msgstr};
    delete $postate->{msgid_1};
    delete $postate->{msgstr_1};
    delete $postate->{context};
    $postate->{type} = "";
  }
}

sub createpos($$@)
{
  my ($data, $keys, @files) = @_;
  foreach my $file (@files)
  {
    my $la;
    if($file =~ /[-_](.._..)\.po$/ || $file =~ /^(?:.*\/)?(.._..)\.po$/ ||
    $file =~ /[-_](..)\.po$/ || $file =~ /^(?:.*\/)?(..)\.po$/)
    {
      $la = $1;
    }
    elsif($file =~ /\.pot$/)
    {
      $la = "en";
    }
    else
    {
      die "Language for file $file unknown.";
    }
    die "Could not open outfile $file\n" if !open FILE,">:utf8",$file;
    print FILE "msgid \"\"\nmsgstr \"\"\n";
    foreach my $k (sort keys %{$keys})
    {
      print FILE "\"$keys->{$k}\\n\"\n";
    }
    print FILE "\n";

    foreach my $en (keys %{$data})
    {
      my $ctx;
      $ctx = $1 if $en =~ s/^___(.*)___//;
      my $str = ($la ne "en" && exists($data->{$en}{$la})) ? $data->{$en}{$la} : "";
      if($data->{$en}{_file})
      {
        foreach my $f (split ";",$data->{$en}{_file})
        {
          print FILE "#: $f\n"
        }
      }
      else
      {
        next;
        # print FILE "#: unknown:0\n"
      }
      print FILE "msgctxt \"$ctx\"\n" if $ctx;
      print FILE "msgid \"$en\"\n";
      print FILE "msgid_plural \"$data->{$en}{\"en.1\"}\"\n" if $data->{$en}{"en.1"};
      if($data->{$en}{"$la.1"})
      {
        print FILE "msgstr[0] \"$str\"\n";
        print FILE "msgstr[1] \"$data->{$en}{\"$la.1\"}\"\n" if $data->{$en}{"$la.1"};
        print FILE "msgstr[2] \"$data->{$en}{\"$la.2\"}\"\n" if $data->{$en}{"$la.2"};
        print FILE "msgstr[3] \"$data->{$en}{\"$la.3\"}\"\n" if $data->{$en}{"$la.3"};
      }
      else
      {
        print FILE "msgstr \"$str\"\n";
      }
      print FILE "\n";
    }
    close FILE;
  }
}

sub maketxt($)
{
  my ($str) = @_;
  $str =~ s/&gt;/>/g;
  $str =~ s/&lt;/</g;
  $str =~ s/"/\\"/g;
  $str =~ s/&quot;/\\"/g;
  $str =~ s/\n/\\n/g;
  return $str;
}

sub makexml($)
{
  my ($str) = @_;
  $str =~ s/</&lt;/g;
  $str =~ s/>/&gt;/g;
  $str =~ s/\\"/&quot;/g;
  $str =~ s/\\n/\n/g;
  return $str;
}

sub replacemat($$$$)
{
  my ($start,$en,$end,$data) = @_;
  $en = maketxt($en);
  my $repl = "$start<desCRIPtion locale=\"en\" >".makexml($en)."</desCRIPtion>$end";
  foreach my $l (sort keys %{$data->{$en}})
  {
    next if $l =~ /[._]/;
    $repl .= "$start<desCRIPtion locale=\"$l\" >".makexml($data->{$en}{$l})."</desCRIPtion>$end";
  }
  return $repl;
}

sub createmat($@)
{
  my ($data, @files) = @_;

  foreach my $file (@files)
  {
    my $x = $/;
    undef $/;
    die "Could not open $file\n" if !open FILE,"<:utf8",$file;
    my $content = <FILE>;
    close FILE;
    foreach my $en (keys %{$data})
    {
      my $ostr = qr/( *)<description +locale="en" *>(.*)<\/description>([\r\n]+)(?: *<description .*\/description>[\r\n]+)*/;
      $content =~ s/$ostr/replacemat($1,$2,$3,$data)/eg;
    }
    if($content =~ /(<description .*)/)
    {
      die "Could not handle string $1.";
    }

    $content =~ s/desCRIPtion/description/g;

    die "Could not open output $file\n" if !open FILE,">:utf8",$file;
    print FILE $content;
    close FILE;
  }
}

sub createts($@)
{
  my ($data, @files) = @_;

  foreach my $file (@files)
  {
    my $x = $/;
    undef $/;
    die "Could not open $file\n" if !open FILE,"<:utf8",$file;
    my $content = <FILE>;
    close FILE;
    foreach my $en (keys %{$data})
    {
#TODO
    }

    die "Could not open output $file\n" if !open FILE,">:utf8",$file;
    print FILE $content;
    close FILE;
  }
}

sub main
{
  my %lang;
  my @mat;
  my @po;
  my @ts;
  foreach my $f (@ARGV)
  {
    if($f =~ /\.mat$/) { push(@mat, $f); }
    elsif($f =~ /\.po$/) { push(@po, $f); }
    elsif($f =~ /\.ts$/) { push(@ts, $f); }
  }
  my %data = loadfiles(\%lang,\%pokeys, @mat,@ts,@po);
  my @cpo;
  my $basename = "templates";
  foreach my $la (sort keys %lang)
  {
    push(@cpo, "${basename}_$la.po");
  }
  push(@cpo, "$basename.pot");
  createpos(\%data, \%pokeys, @cpo);

  createmat(\%data, @mat) if @mat;
  createts(\%data, @mat) if @ts;
}