#pragma once

#include <string>
#include <vector>
#include <map>
#include <cmath>

#include "moses/InputType.h"
#include "moses/Range.h"
#include "moses/TargetPhrase.h"

#include "ConfusionSet.h"

namespace Moses
{

// Represents the position of a confusion word in a vector of strings. (-1, -1)
// means it is not set, and (i, i-1) indicates a <null> confusion word.
struct CWordPos {
  int i;
  int j;

  CWordPos() : i(-1), j(-1) { }
  CWordPos(int i, int j) : i(i), j(j) { }

  inline bool IsSet() const {
    return i >= 0;
  }

  inline bool IsNull() const {
    return i == j+1;
  }

  inline size_t Size() const {
    return i - j + 1;
  }
};

inline std::ostream& operator << (std::ostream& out, const CWordPos& pos)
{
  out << "(" << pos.i << "," << pos.j << ")";
  return out;
}

// Keeps results of translation option analysis.
struct CWordInfo {
  // position of confusion word in the source phrase
  CWordPos sourcePos;
  // positions of confusion words in specified targets
  std::map<size_t, CWordPos> targetPos;

  inline bool IsFound() const {
    return ! targetPos.empty();
  }
  inline bool FoundInTarget(size_t idx) const {
    return targetPos.find(idx) != targetPos.end();
  }
};

inline std::ostream& operator << (std::ostream& out, const CWordInfo& info)
{
  out << "Source: " << info.sourcePos << " Targets: ";
  for (std::map<size_t, CWordPos>::const_iterator it = info.targetPos.begin(); it != info.targetPos.end(); ++it)
    out << "[" << it->first << "]: " << it->second << ", ";
  return out;
}

class ConfusionWordFinder
{
public:
  ConfusionWordFinder(const Discriminative::ConfusionSet& confusionSet)
    : m_confusionSet(confusionSet) { }

  ~ConfusionWordFinder() { }

  CWordInfo AnalyzeTranslationOptions(
        const InputType& input
      , const Range& sourceRange
      , const TranslationOptionList &translationOptionList) {

    CWordInfo info;
    if (m_confusionSet.Empty())
      return info;

    // get source string without confusion words
    int sourcePosStart = -1, sourcePosEnd = -1;
    std::string sourceStr = GetSourceWithoutConfusionWords(input, sourceRange,
      sourcePosStart, sourcePosEnd);

    int nullToNullSubIdx = -1;

    // compare target phrases with source
    for (size_t toptIdx = 0; toptIdx < translationOptionList.size(); toptIdx++) {
      const TargetPhrase& targetPhrase =
        translationOptionList.Get(toptIdx)->GetTargetPhrase();

      // we are interested only in a single word edition, so targets differs in
      // more than one word in length can be skipped
      if (abs(sourceRange.GetNumWordsCovered() - targetPhrase.GetSize()) > 1)
        continue;

      // get target string without confusion words
      int targetPosStart = -1, targetPosEnd = -1;
      std::string targetStr = GetTargetWithoutConfusionWords(targetPhrase,
        targetPosStart, targetPosEnd);

      //std::cerr << "      CS [" << toptIdx << "] :: " << sourceStr << " (" << sourcePosStart << "," << sourcePosEnd << ") == "
        //<< targetStr << " (" << targetPosStart << "," << targetPosEnd << ")\n";

      // compare source and target strings to check if there is an insertion or
      // deletion of the confusion word;
      // a range in the form of (i, i-1) indicates the <null> confusion word
      if (sourceStr == targetStr) {
        // handle word substitution
        if (sourcePosStart > -1 && sourcePosStart == targetPosStart) {
          info.targetPos[toptIdx] = CWordPos(targetPosStart, targetPosEnd);
        }
        // handle word deletion
        else if (sourcePosStart > -1 && targetPosStart == -1) {
          info.targetPos[toptIdx] = CWordPos(sourcePosStart, sourcePosEnd - 1);
        }
        // handle word insertion
        else if (sourcePosStart == -1 && targetPosStart > -1) {
          info.targetPos[toptIdx] = CWordPos(targetPosStart, targetPosEnd);
        }
        // handle <null> -> <null> substitution
        // we made a naive assumption that there is only one such target
        else if (sourcePosStart == -1 && targetPosStart == -1) {
          nullToNullSubIdx = toptIdx;
        }
      }
    }

    if (! info.targetPos.empty()) {
      // if the source position is not set, update it with the position of a
      // random target (e.g. the first one)
      if (sourcePosStart == -1) {
        // TODO: make it better, for example, take the target with the best score
        //
        //size_t targetWithBestScore = 0;
        //float bestScore = LOWEST_SCORE;
        //for (size_t i = 0; i < info.targetPos.size(); ++i)
          //break;

        const CWordPos& pos = info.targetPos.begin()->second;
        info.sourcePos = CWordPos(pos.i, pos.i - 1);
      } else {
        info.sourcePos = CWordPos(sourcePosStart, sourcePosEnd);
      }

      // update the target with <null> to <null> substitution
      if (nullToNullSubIdx != -1) {
        info.targetPos[nullToNullSubIdx] = info.sourcePos;
      }
    }

    return info;
  }

private:

  std::string GetSourceWithoutConfusionWords(const InputType& input
                                           , const Range& sourceRange
                                           , int& sourcePosStart
                                           , int& sourcePosEnd) const {
    size_t startIdx = sourceRange.GetStartPos();
    size_t endIdx = sourceRange.GetEndPos();
    std::string result;

    for (size_t i = startIdx; i <= endIdx; ++i) {
      if (! m_confusionSet.Has(GetSourceWord(input, i))) {
        if (! result.empty())
          result += " ";
        result += GetSourceWord(input, i);
      } else {
        // only firstly encountered confusion word is taken into consideration
        if (sourcePosStart == -1) {
          sourcePosStart = i - startIdx;
          sourcePosEnd = i - startIdx;
        }
      }
    }
    return result;
  }

  std::string GetTargetWithoutConfusionWords(const TargetPhrase& phrase
                                           , int& targetPosStart
                                           , int& targetPosEnd) const {
    std::string result;
    for (size_t i = 0; i < phrase.GetSize(); ++i) {
      if (! m_confusionSet.Has(GetTargetWord(phrase, i))) {
        if (! result.empty())
          result += " ";
        result += GetTargetWord(phrase, i);
      } else {
        // only firstly encountered confusion word is taken into consideration
        if (targetPosStart == -1) {
          targetPosStart = i;
          targetPosEnd = i;
        }
      }
    }
    return result;
  }

  inline std::string GetSourceWord(const InputType& input, size_t pos) const {
    return input.GetWord(pos).GetString(0).as_string();
  }
  inline std::string GetTargetWord(const TargetPhrase &phrase, size_t pos) const {
    return phrase.GetWord(pos).GetString(0).as_string();
  }

  const Discriminative::ConfusionSet& m_confusionSet;

};

} // namespace Moses
