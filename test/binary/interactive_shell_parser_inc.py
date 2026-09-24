#!/usr/bin/env python3
###############################################################################
# This file is part of the cvc5 project.
#
# Copyright (c) 2009-2026 by the authors listed in the file AUTHORS
# in the top-level source directory and their institutional affiliations.
# All rights reserved.  See the file COPYING in the top-level source
# directory for licensing information.
# #############################################################################
#
# A simple test file to interact with ava6 via the interactive shell in
# incremental mode.
##

import sys
import pexpect

def error_message(s):
    return "Unexpected output '" + s + "'"

def expect_exact(child, s):
    child.expect_exact(s)
    assert child.before == b"", error_message(child.before.decode('UTF-8'))
    assert child.after == s.encode('UTF-8'), error_message(child.after)

def sendline(child, s):
    child.sendline(s)
    expect_exact(child, s+'\r\n')

def check_iteractive_shell_parser_inc():
    """
    Interacts with ava6's interactive shell and checks that parser declarations are
    managed properly in incremental mode.
    """

    # Open ava6
    child = pexpect.spawn("bin/ava6", timeout=1)

    # We expect to see the ava6 prompt
    child.expect("ava6> ")
    sendline(child, "(set-logic ALL)")
    expect_exact(child, "ava6> ")
    sendline(child,"(set-option :incremental true)")
    expect_exact(child,"ava6> ")
    sendline(child,"(push)")
    expect_exact(child,"ava6> ")
    sendline(child,"(declare-fun x () Int)")
    expect_exact(child,"ava6> ")
    sendline(child,"(check-sat)")
    expect_exact(child,"sat\r\nava6> ")
    sendline(child,"(pop)")
    expect_exact(child,"ava6> ")
    sendline(child,"(declare-fun x () Int)")
    expect_exact(child,"ava6> ")
    sendline(child,"(check-sat)")
    expect_exact(child,"sat\r\nava6> ")

    return 0


def main():
    """
    Runs our interactive shell test

    Caveats:

        * If we don't have the "pexpect" model, the test doesn't get run, but
          passes

        * We expect pexpect to raise and exit with a non-zero exit code if any
          of the steps fail
    """

    # If any of the "steps" fail, the pexpect will raise a Python will exit
    # with a non-zero error code
    sys.exit(check_iteractive_shell_parser_inc())

if __name__ == "__main__":
    main()

# EOF
