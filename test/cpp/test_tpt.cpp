#include <base.h>
#include <mine.h>
#include <qminer.h>

#include "microtest.h"


// Test parent class

ClassTP(TTestParent, PTestParent)//{
private:
    UndefCopyAssign(TTestParent);
protected:
    TTestParent() {}
public:
    virtual ~TTestParent() {}
    static PTestParent New() { return new TTestParent(); }
};

TEST(TPtParent) {
    PTestParent Parent = TTestParent::New();
}

// Test child class

ClassTPE(TTestChild, PTestChild, TTestParent)//{
private:
    UndefCopyAssign(TTestChild);
    TTestChild() {}
public:
    static PTestChild New() { return new TTestChild(); }
    static PTestParent NewAsParent() { return new TTestChild(); }
};

TEST(TPtChild) {
    PTestChild Child = TTestChild::New();
    PTestParent ChildAsParent = TTestChild::NewAsParent();
}

TEST(TPtParentFromChild) {
    PTestChild Child = TTestChild::New();
    PTestParent ParentFromChild = Child();
}

// Test multiple-inheritance child class

ClassT(TTestParentOther)//{
private:
    UndefCopyAssign(TTestParentOther);
protected:
    TTestParentOther() {}
public:
    virtual ~TTestParentOther() {}
};

ClassTPEE(TTestChildMulti, PTestChildMulti, TTestParent, TTestParentOther)//{
private:
    UndefCopyAssign(TTestChildMulti);
    TTestChildMulti() {}
public:
    static PTestChildMulti New() { return new TTestChildMulti(); }
    static PTestParent NewAsParent() { return new TTestChildMulti(); }
};

TEST(TPtChildMulti) {
    PTestChildMulti Child = TTestChildMulti::New();
    PTestParent ChildAsParent = TTestChildMulti::NewAsParent();
}

TEST(TPtParentFromChildMulti) {
    PTestChildMulti Child = TTestChildMulti::New();
    PTestParent ParentFromChild = Child();
}