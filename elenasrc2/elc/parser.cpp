//---------------------------------------------------------------------------
//		E L E N A   P r o j e c t:  ELENA Compiler
//
//		This file contains ELENA Parser class implementation.
//
//                                              (C)2005-2018, by Alexei Rakov
//---------------------------------------------------------------------------

#include "elena.h"
// --------------------------------------------------------------------------
#include "parser.h"
#include "errors.h"

using namespace _ELENA_ ;

// --- SyntaxError ---

SyntaxError :: SyntaxError(int column, int row, ident_t token)
{
   this->error = errInvalidSyntax;
   this->column = column;
   this->row = row;
   this->token = token;
}

SyntaxError :: SyntaxError(int column, int row, ident_t token, const char* error)
{
   this->error = error;
   this->column = column;
   this->row = row;
   this->token = token;
}

inline const char* getError(int symbol)
{
   switch(symbol) {
   //   case nsDeclarationEndExpected:
   //   case nsStatementEndExpected:
   //   case nsDirectiveEndExpected:
   //   case nsInlineExpressionEndExpected:
   //      return errDotExpectedSyntax;
   //   case nsErrClosingSBracketExpected:
   //      return errCSBrExpectedSyntax;
   //   //case nsErrNestedMemberExpected:
   //   //   return errMethodNameExpected;
   //   //case nsErrObjectExpected:
   //   //   return errObjectExpected;
   //   //case nsErrMessageExpected:
   //   //   return errMessageExpected;
      default:
         return errInvalidSyntax;
   }
}

LexicalType getLexicalType(int symbol)
{
   // NOTE : the last 8 bits are used as a token variant
   return (LexicalType)(symbol & 0xFFFFFFF0);
}

TerminalInfo getTerminalInfo(ParserTable& table, LineInfo info)
{
   TerminalInfo terminal;
   terminal.value = info.line;
   terminal.col = info.column;
   terminal.row = info.row;
   terminal.disp = info.position;
   terminal.length = info.length;
   switch (info.state) {
//      case dfaQuote:
//         terminal.symbol = tsLiteral;
//         break;
//      case dfaCharacter:
//         terminal.symbol = tsCharacter;
//         break;
      case dfaEOF:
         terminal.symbol = lxEOF;
         terminal.value = _eof_message;
         break;
      case dfaIdentifier:
         terminal.symbol = lxIdentifier;
         break;
      case dfaFullIdentifier:
         terminal.symbol = lxReference;
         break;
      case dfaGlobal:
         terminal.symbol = lxGlobalReference;
         break;
//    //  case dfaPrivate:
//    //     if (terminal.value[1] == 0) {
//    //        //HOTFIX : if it is $ symbol
//    //        terminal.symbol = (Symbol)table.defineSymbol(terminal);
//    //     }
//    //     else terminal.symbol = tsPrivate;
//    //     break;
//      case dfaInteger:
//         terminal.symbol = tsInteger;
//         break;
      case dfaExplicitConst:
         switch (terminal.value[getlength(terminal.value) - 1]) {
            case 'h':
               terminal.symbol = lxHexInteger;
               break;
//            case 'l':
//               terminal.symbol = tsLong;
//               break;
//            case 'r':
//               terminal.symbol = tsReal;
//               break;
//            default:
//               terminal.symbol = tsExplicitConst;
//               break;
         }
         break;
//      case dfaLong:
//         terminal.symbol = tsLong;
//         break;
	   case dfaHexInteger:
         terminal.symbol = lxHexInteger;
         break;
//	   case dfaReal:
//         terminal.symbol = tsReal;
//         break;
//      case dfaWideQuote:
//         terminal.symbol = tsWide;
//         break;
//      case dfaAttribute:
      default:
         terminal.symbol = (LexicalType)(table.defineSymbol(terminal));
   }
   return terminal;
}

// --- Parser ---

Parser :: Parser(StreamReader* syntax)
{
   _table.load(syntax);
}

bool Parser :: derive(TerminalInfo& terminal, ParserStack& stack, _DerivationWriter& writer, bool& traceble)
{
   int current = stack.pop();
   while (!test(current, mskTerminal)) {
      traceble = test(current, mskTraceble);
      if (current == 0)
         writer.closeNode();
      else {
         if (traceble) {
            stack.push(0);
            writer.newNode(getLexicalType(current));
         }
         if (!_table.read(current, terminal.symbol, stack))
            return false;

         if (test(current, mskError))
            throw SyntaxError(terminal.Col(), terminal.Row(), terminal.value, getError(current));
      }
      current = stack.pop();
   }
   return (terminal == current);
}

void Parser :: parse(TextReader* reader, _DerivationWriter& writer, int tabSize)
{
   SourceReader source(tabSize, reader);
   ParserStack  stack(lxEOF);
   TerminalInfo terminal;

   stack.push(nsStart);
   do {
      terminal = getTerminalInfo(_table, source.read(_buffer, IDENTIFIER_LEN));

      bool traceble = false;
      if (!derive(terminal, stack, writer, traceble))
         throw SyntaxError(terminal.Col(), terminal.Row(), _buffer);

      if (traceble)
         writer.appendTerminal(terminal);

   } while (terminal != lxEOF);
}
