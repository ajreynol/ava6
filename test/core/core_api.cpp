#include <ava6/ava6.h>
#include <ava6/ava6_parser.h>

#include <iostream>
#include <stdexcept>

using namespace ava6;

static void require(bool condition, const char* message)
{
  if (!condition) throw std::runtime_error(message);
}

int main()
{
  try
  {
    TermManager tm;
    Solver solver(tm);
    for (const auto& name : solver.getOptionNames())
    {
      require(solver.getOptionInfo(name).category == modes::OptionCategory::COMMON
                  || solver.getOptionInfo(name).category == modes::OptionCategory::REGULAR
                  || solver.getOptionInfo(name).category == modes::OptionCategory::UNDOCUMENTED,
              "Expert option remains exposed");
    }
    solver.setLogic("QF_LIA");
    solver.setOption("produce-models", "true");
    solver.setOption("produce-proofs", "true");
    solver.setOption("incremental", "true");
    Term x = tm.mkConst(tm.getIntegerSort(), "x");
    solver.assertFormula(tm.mkTerm(Kind::EQUAL, {x, tm.mkInteger(7)}));
    require(solver.checkSat().isSat(), "Expected sat");
    require(solver.getValue(x) == tm.mkInteger(7), "Incorrect model");
    solver.push();
    solver.assertFormula(tm.mkTerm(Kind::EQUAL, {x, tm.mkInteger(8)}));
    require(solver.checkSat().isUnsat(), "Expected unsat");
    auto proofs = solver.getProof();
    require(!proofs.empty(), "Missing proof");
    std::string cpc = solver.proofToString(proofs.front(), modes::ProofFormat::CPC);
    require(cpc.find("step") != std::string::npos, "Missing CPC proof steps");
    solver.pop();
    require(solver.checkSat().isSat(), "Broken incremental context");

    Solver parsed(tm);
    parser::InputParser parser(&parsed);
    parser.setStringInput(modes::InputLanguage::SMT_LIB_2_6,
                          "(set-logic QF_UF) (declare-const p Bool) (assert p)",
                          "core-api");
    while (true)
    {
      parser::Command command = parser.nextCommand();
      if (command.isNull()) break;
      command.invoke(&parsed, parser.getSymbolManager(), std::cout);
    }
    require(parsed.checkSat().isSat(), "C++ parser integration failed");
    std::cout << "PASS C++ API: options, models, CPC, contexts, parser\n";
    return 0;
  }
  catch (const std::exception& e)
  {
    std::cerr << e.what() << '\n';
    return 1;
  }
}
