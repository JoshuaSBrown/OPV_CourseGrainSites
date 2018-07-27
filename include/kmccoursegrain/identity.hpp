#ifndef KMCCOURSEGRAIN_IDENTITY_H_
#define KMCCOURSEGRAIN_IDENTITY_H_

#include <exception>

namespace kmccoursegrain {

  class Identity {
    private:
      int id_;
      bool id_set_;

    public:
      Identity() : id_set_(false) {}
      int getId() {
        return (id_set_ ? id_ : throw std::runtime_error("ID not set"));
      }
      void setId( int id) { id_ = id; id_set_ = true;}
  };
}

#endif // KMCCOURSEGRAIN_IDENTITY_H_