#ifndef MYTHICAL_IDENTITY_HPP
#define MYTHICAL_IDENTITY_HPP

#include <stdexcept>

namespace mythical {

/**
 * \brief Class to constrol the use of the identity method
 *
 * This class is meant to be inherited by any class that requires an id, thus it
 * will ensure that the methods are uniform.
 **/
class Identity {
 private:
  int id_;
  bool id_set_;

 public:
  Identity() : id_set_(false) {}
  virtual ~Identity() {};
  int getId() const {
    return (id_set_ ? id_ : throw std::runtime_error("ID not set"));
  }
  void setId(int id) {
    id_ = id;
    id_set_ = true;
  }
};
}

#endif  // MYTHICAL_IDENTITY_HPP
