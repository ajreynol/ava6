###############################################################################
# This file is part of the cvc5 project.
#
# Copyright (c) 2009-2026 by the authors listed in the file AUTHORS
# in the top-level source directory and their institutional affiliations.
# All rights reserved.  See the file COPYING in the top-level source
# directory for licensing information.
# #############################################################################
##

import pytest
import ava6

from ava6 import InputParser, SymbolManager

@pytest.fixture
def tm():
    return ava6.TermManager()
@pytest.fixture
def solver(tm):
    return ava6.Solver(tm)

def parse_command(solver, sm, cmd_str):
    parser = InputParser(solver, sm)
    parser.setIncrementalStringInput(ava6.InputLanguage.SMT_LIB_2_6, "test_command")
    parser.appendIncrementalStringInput(cmd_str + '\n')
    return parser.nextCommand()


def test_invoke(tm, solver):
    sm = SymbolManager(tm)
    cmd = parse_command(solver, sm, "(set-logic QF_LIA)")
    assert cmd.isNull() is not True
    cmd.invoke(solver, sm)
    # get model not available
    cmd = parse_command(solver, sm, "(get-model)")
    assert cmd.isNull() is not True
    cmd.invoke(solver, sm)
    # logic already set
    with pytest.raises(RuntimeError):
        parse_command(solver, sm, "(set-logic QF_LRA)")

def test_to_string(tm, solver):
    sm = SymbolManager(tm)
    cmd = parse_command(solver, sm, "(set-logic QF_LIA )")
    assert cmd.isNull() is not True
    # note normalizes wrt whitespace
    assert cmd.toString() == "(set-logic QF_LIA)"

def test_get_command_name(tm, solver):
    sm = SymbolManager(tm)
    cmd = parse_command(solver, sm, "(get-model)")
    assert cmd.isNull() is not True
    assert cmd.getCommandName() == "get-model"
