#ifndef moses_ConfusionSet_h
#define moses_ConfusionSet_h

#include <set>
#include <string>

#include <boost/algorithm/string.hpp>

#include "../moses/Util.h"

namespace Discriminative
{

class ConfusionSet
{
public:
  ConfusionSet() {}
  ConfusionSet(const std::set<std::string>& cWords)
    : m_confusionWords(cWords) {}

  virtual ~ConfusionSet() {}

  void Reset(const std::set<std::string>& cWords) {
    m_confusionWords.clear();
    m_confusionWords = cWords;
  }

  bool Has(const std::string& word) const {
    //FIXME
    //std::cerr << "    CSet :: " << ToString() << " :: Has " << word << "? =" << result << "\n";
    return m_confusionWords.find(word) != m_confusionWords.end();
  }

  bool Empty() const {
    return m_confusionWords.empty();
  }

  std::string ToString() const {
    return Moses::Join(",", m_confusionWords.begin(), m_confusionWords.end());
  }

private:
  std::set<std::string> m_confusionWords;
};


} // namespace Discriminative

#endif // moses_ConfusionSet_h
