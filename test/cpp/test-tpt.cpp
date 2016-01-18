#include <gtest/gtest.h>

#include <base.h>

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

TEST(TPt, Parent) {
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

TEST(TPt, Child) {
    PTestChild Child = TTestChild::New();
    PTestParent ChildAsParent = TTestChild::NewAsParent();
}

TEST(TPt, ParentFromChild) {
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

TEST(TPt, ChildMulti) {
    PTestChildMulti Child = TTestChildMulti::New();
    PTestParent ChildAsParent = TTestChildMulti::NewAsParent();
}

TEST(TPt, ParentFromChildMulti) {
    PTestChildMulti Child = TTestChildMulti::New();
    PTestParent ParentFromChild = Child();
}
