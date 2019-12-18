#!/usr/bin/perl

# ttydriver "XXX";
# ttymodel "YYY";
# ttycode "ZZ";
# manufacturer "XXX";
# model "YYY";
# display {
#   row {
#     cells 8;
#     router1 {
#       router GRP; # name 
#     }
#   }
#   button NUM GRP ID "name";
#   # untranslated key group GRP "name"
# }

my $driver = undef;
my $model = undef;
my $code = undef;
my @entries = undef;

sub CreateFile
{
    open(FILE, ">mg-definitions/$code-$model") or die "Error writing file";
    print FILE qq(ttydriver    "$driver"\n);
    print FILE qq(ttymodel     "$model"\n);
    print FILE qq(ttycode      "$code"\n);
    print FILE qq(manufacturer "$driver"\n);
    my $Model = ucfirst($model);
    print FILE qq(model        "$Model"\n\n);
    print FILE qq(display {\n);

    print FILE qq(  row {\n);
    print FILE qq(    cells 8;\n);
    foreach (@entries)
    {
        if ($_ && @$_[2] =~ m/Routing/)
        {
            my $level = 1;
            if (@$_[2] =~ m/(\d)/)
            {
                $level = $1;
            }
            print FILE qq(    router$level {\n);
            print FILE qq(      router   @$_[0];  # @$_[2]\n);
            print FILE qq(    }\n);
            $_ = undef;
        }
    }
    print FILE qq(  }\n\n);

    my $I = 1;
    foreach (@entries)
    {
        if ($_)
        {
            if (@$_[1] < 0)
            {
                print FILE qq(  # unrecognized group @$_[0] named "@$_[2]"\n);
            }
            elsif (@$_[2] =~ m/Dot(\d)/)
            {
                print FILE qq(  dot$1   @$_[0] @$_[1]; # @$_[2]\n);
            }
            elsif (@$_[2] eq 'LeftSpace' || @$_[2] eq 'SpaceLeft')
            {
                print FILE qq(  lspace   @$_[0] @$_[1]; # @$_[2]\n);
            }
            elsif (@$_[2] eq 'RightSpace' || @$_[2] eq 'SpaceRight')
            {
                print FILE qq(  rspace   @$_[0] @$_[1]; # @$_[2]\n);
            }
            elsif (@$_[2] eq 'Space')
            {
                print FILE qq(  space   @$_[0] @$_[1]; # @$_[2]\n);
            }
            else
            {
                print FILE qq(  button $I   @$_[0] @$_[1] "@$_[2]";\n);
                ++$I;
            }
        }
    }
    print FILE qq(}\n);
    close FILE;
}

open (ALLTABLES, "<alltables") or die "Can't find alltables";
while (<ALLTABLES>)
{
    if (/DRIVER_CODE = (.*)/)
    {
        $code = $1;
    }
    elsif (/DRIVER_NAME = (.*)/)
    {
        $driver = $1;
    }
    elsif (/table (.*) \{/)
    {
        $model = $1;
    }
    elsif (/entry (.*) (.*) \[(.*)\]/)
    {
        push @entries, [$1,$2,$3];
    }
    elsif (/\}/)
    {
        CreateFile;
        @entries = undef;
        $model = undef;
    }
}