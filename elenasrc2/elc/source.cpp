//---------------------------------------------------------------------------
//		E L E N A   P r o j e c t:  ELENA Compiler
//
//		This file contains ELENA Source Reader class implementation.
//
//                                              (C)2005-2019, by Alexei Rakov
//---------------------------------------------------------------------------

#include "elena.h"
// --------------------------------------------------------------------------
#include "source.h"

using namespace _ELENA_ ;

// --- DFA Table ---

const char* DFA_table[35] =
{
        ".????????CC??C??????????????????CMVBXHMEHHMMHYHINNNNNNNNNNMMMM`MaDDDDDDDDDDDDDDDDDDDDDDDDDDcMHMD?DDDDDDDDDDDDDDDDDDDDDDDDDDHMHHD",
        "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA??????????AAAAAAABBBBBBBBBBBBBBBBBBBBBBBBBBAAAAAABBBBBBBBBBBBBBBBBBBBBBBBBBABAB]",
        "*********CC*********************C***********************************************************************************************",
        "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAEAAAAAAAADDDDDDDDDDAAAAAAADDDDDDDDDDDDDDDDDDDDDDDDDDAAAADADDDDDDDDDDDDDDDDDDDDDDDDDDAAAAD",
        "??????????????????????????????????????????G?????FFFFFFFFFF???????FFFFFFFFFFFFFFFFFFFFFFFFFF????F?FFFFFFFFFFFFFFFFFFFFFFFFFF????F",
        "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAEAAAAAAAAFFFFFFFFFFAAAAAAAFFFFFFFFFFFFFFFFFFFFFFFFFFAAAAFAFFFFFFFFFFFFFFFFFFFFFFFFFFAAAAF",
        "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA?AA?AAAAA??????????AAAAAAA??????????????????????????AAAA?A??????????????????????????AAAA?",
        "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA",
        "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAKAAAAJAAAAAAAAAAAAAHAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA",
        "*JJJJJJJJJ*JJ*JJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJ",
        "?KKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKLKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKK",
        "?KKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKCKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKK",
        "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAHAAAAAAAAAAAAAAAAAAAHHHHHHAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAHAAAAAAAAAAAAAAAAAAAAAAAAAAAAAHAAA",
        "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA$ANNNNNNNNNNAAAAAAASSSSSSSSSSSSSSSSSSSSSSSSSSAAAAAASSSSSSSSSSSSSSSSSSSSSSSSSSAAAAA",
        "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!PPPPPPPPPP!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!",
        "??????????????????????????????????????????????" "!?PPPPPPPPPP???????????????????????????????????????????Z????????????Q?????????????", //!! the space should be removed after turning off trigraphs
        "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA",
        "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA",
        "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAASSSSSSSSSSAAAAAAASSSSSSATAAAAAAAAAAAAAAAAAAAAAAAASSSSSSATAAARAAAAAAAAAAAAAAAAAAA",
        "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA",
        "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!NNNNNNNNNN!!!H!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!",
        "?VVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVWVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVV",
        "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAVA]AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA^AAAAAAAA",
        "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA?AAAAAA$A]]]]]]]]]]AAAAAAAXXXXXXXXXXXXXXXXXXXXXXXXXXAAAAXAXXXXXXXXXXXXXXXXXXXXXXXXXXAAAAX",
        "-------------------------------------------------------------H------------------------------------------------------------------",
        "???????????????????????????????????????????_?_??[[[[[[[[[[??????????????????????????????????????????????????????????????????????",
        "????????????????????????????????????????????????[[[[[[[[[[????????????????????????????????????????????????????????Q?????????????",
        "????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????",
        "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAVA]AAAAAAAAAAA]]]]]]]]]]AAAAAAA]]]]]]AAAAAAAAAAAAAAAAAAAAAAAAAA]]]]]]A]AAAAAAAAAAAAAAAAAAAAAAA",
        "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA",
        "????????????????????????????????????????????????[[[[[[[[[[????????????????????????????????????????????????????????Q?????????????",
        "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAHAAAAAAAAAAAAAAAAAAAAHHHAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAHAAAAAAAAAAAAAAAAAAAAAAAAAAAAAHAAA",
        "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAbAAAAAAAAaaaaaaaaaaAAAAAAAaaaaaaaaaaaaaaaaaaaaaaaaaaAAAAaAaaaaaaaaaaaaaaaaaaaaaaaaaaAAAAa",
        "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAbAAAAAAAAbbbbbbbbbbAAAAAAAbbbbbbbbbbbbbbbbbbbbbbbbbbAAAAbAbbbbbbbbbbbbbbbbbbbbbbbbbbAAAAb",
        "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAHAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA",
};

// --- SourceReader ---

SourceReader :: SourceReader(int tabSize, TextReader* source)
   : _TextParser(DFA_table, tabSize, source)
{
   _lastState = dfaStart;
}

LineInfo SourceReader :: read(char* token, size_t length)
{
   LineInfo info(_position, _column, _row);
   char terminalState = readLineInfo(dfaStart, info);

   switch (terminalState) {
      case dfaError:
         throw InvalidChar(info.column, info.row, _line[_position]);
      case dfaEOF:
         info.state = dfaEOF;
         info.length = 0;
         break;
      case dfaDotLookahead:
         resolveDotAmbiguity(info);
         break;
      case dfaMinusLookahead:
         resolveSignAmbiguity(info);
         break;
   }

   if (info.state == dfaQuote || info.state == dfaCharacter || info.state == dfaWideQuote) {
      copyQuote(info);
   }
   else copyToken(info, token, length);

   _lastState = info.state;

   return info;
}
