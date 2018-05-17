//import system'collections.
//
//symbol idleFunc1 = (:x) [].

// === pattern enumerators ==

// --- Toping ---

public class TopFilter :: Enumerator
{
    ref<int>      theOffset.    
    ref<int>      theCounter.    
    T<Enumerator> theEnumerator.
    
    constructor new(int aCounter, Enumerator anEnumerator)
    [
        theCounter := aCounter.
        theOffset := 0.
        theEnumerator := anEnumerator.
    ]
    
    constructor new(int counter, int offset, Enumerator enumerator)
    [
        theCounter := counter.
        theOffset := offset.
        
        theEnumerator := enumerator.
    ]
    
    get = theEnumerator get.
    
    bool next
    [
        if (theCounter != 0)
        [
            while (theEnumerator next)
            [
                if (theOffset == 0)
                    [  
                        theCounter reduce(1). ^ true
                    ];
                    [ theOffset reduce(1) ]
            ].
        ].
        
        ^ false
    ]
    
    reset 
    [
        theEnumerator reset.
    ]    
    
    enumerable => theEnumerator.
}

//// --- SkipFilter ---
//
//class SkipFilter :: Enumerator
//{
//    vint       theCounter.    
//    bool       skipped.    
//    enumerator theEnumerator.
//    
//    constructor new int:aCounter enumerator:anEnumerator
//    [
//        theCounter := aCounter.
//        skipped := false.
//        theEnumerator := anEnumerator.
//    ]
//    
//    get = theEnumerator get.
//    
//    bool next
//    [
//        ifnot (skipped)
//        [
//            skipped := true.
//            
//            int i := theCounter.
//            while (i > 0)            
//            [
//                ifnot (theEnumerator next)
//                [
//                    ^ false
//                ].
//                
//                i -= 1.
//            ]
//        ].
//        
//        ^ theEnumerator next
//    ]
//    
//    reset 
//    [
//        theEnumerator reset.
//        skipped := false.
//    ]    
//}

// --- Filtering ---

public class Filter :: Enumerator
{
    T<Func1>      theFilter.
    T<Enumerator> theEnumerator.

    constructor new(Func1 aSelector, Enumerator anEnumerator)
    [
        theFilter := aSelector.
        theEnumerator := anEnumerator.
    ]
    
    constructor new(ExtensionMessage anExtension, Enumerator anEnumerator)
    [
        theFilter := (:x)[ ^ x~anExtension eval ].
        theEnumerator := anEnumerator.
    ]
    
    get = theEnumerator get.
    
    bool next
    [
        while (theEnumerator next)
        [
            if (theFilter(theEnumerator get))
                [ ^ true ]
        ].
        
        ^ false
    ]
    
    reset 
    [
        theEnumerator reset.
    ]    
    
    enumerable => theEnumerator.
}

// --- PatternFilter2 ---

public class PatternFilter2 :: Enumerator
{
    T<Func2>      theFilter.
    T<Enumerator> theEnumerator.
    object        thePrevious.
    
    constructor new(Func2 aFilter, Enumerator anEnumerator)
    [
        theFilter := aFilter.
        theEnumerator := anEnumerator.
    ]
    
    get = theEnumerator get.
    
    bool next
    [
        if (nil == thePrevious)
        [
            ifnot (theEnumerator next)
                [ ^ false ].
            
            thePrevious := theEnumerator get.
        ].
        
        while (theEnumerator next)
        [
            if (theFilter(thePrevious,theEnumerator get))
                [ ^ true ].
            
            thePrevious := theEnumerator get
        ].
        
        ^ false
    ]
    
    reset
    [
        theEnumerator reset.
        thePrevious := nil.
    ]
    
    enumerable => theEnumerator.
}

// --- PatternFilter3 ---

public class PatternFilter3 :: Enumerator
{
    T<Func3>      theFilter.
    T<Enumerator> theEnumerator.
    object        thePrevious.
    object        theBeforePrev.
    
    constructor new(Func3 aFilter, Enumerator anEnumerator)
    [
        theFilter := aFilter.
        theEnumerator := anEnumerator.
    ]
    
    get = theEnumerator get.
    
    reset
    [
        theEnumerator reset.
        thePrevious := nil.
    ]
    
    bool next
    [
        if (nil == thePrevious)
        [
            ifnot (theEnumerator next)
                [ ^ false ].
            
            theBeforePrev := theEnumerator get.
    
            ifnot (theEnumerator next)
                [ ^ false ].
            
            thePrevious := theEnumerator get
        ].
                
        while (theEnumerator next)
        [
            if(theFilter(theBeforePrev,thePrevious,theEnumerator get))
                [ ^ true ].
            
            theBeforePrev := thePrevious.
            thePrevious := theEnumerator get
        ].
        
        ^ false
    ]
    
    enumerable => theEnumerator.
}

// --- PatternFilter4 ---

public class PatternFilter4 :: Enumerator
{
    T<Func4>      theFilter.
    T<Enumerator> theEnumerator.
    object        thePrevious.
    object        theBeforePrev.
    object        theBefore2Prev.
    
    constructor new(Func4 aFilter, Enumerator anEnumerator)
    [
        theFilter := aFilter.
        theEnumerator := anEnumerator.
    ]
    
    get = theEnumerator get.
    
    reset
    [
        theEnumerator reset.
        thePrevious := nil.
    ]
    
    bool next
    [
        if(nil == thePrevious)
        [
            ifnot(theEnumerator next)
                [ ^ false ].
            
            theBefore2Prev := theEnumerator get.
            
            ifnot(theEnumerator next)
                [ ^ false ].
            
            theBeforePrev := theEnumerator get.
    
            ifnot(theEnumerator next)
                [ ^ false ].
            
            thePrevious := theEnumerator get.
        ].
                
        while (theEnumerator next)
        [
            if (theFilter(theBefore2Prev,theBeforePrev,thePrevious,theEnumerator get))
                [ ^ true ].
            
            theBefore2Prev := theBeforePrev.
            theBeforePrev := thePrevious.
            thePrevious := theEnumerator get
        ].
        
        ^ false.
    ]
    
    enumerable => theEnumerator.
}

// --- PatternFilter5 ---

public class PatternFilter5 :: Enumerator
{
    T<Func5>      theFilter.
    T<Enumerator> theEnumerator.
    object        thePrevious.
    object        theBeforePrev.
    object        theBefore2Prev.
    object        theBefore3Prev.
    
    constructor new(Func5 aFilter, Enumerator anEnumerator)
    [
        theFilter := aFilter.
        theEnumerator := anEnumerator.
    ]
    
    get = theEnumerator get.
    
    reset
    [
        theEnumerator reset.
        thePrevious := nil.
    ]
    
    bool next
    [
        if (nil == thePrevious)
        [
            ifnot (theEnumerator next)
                [ ^ false ].
            
            theBefore3Prev := theEnumerator get.
            
            ifnot(theEnumerator next)
                [ ^ false ].
            
            theBefore2Prev := theEnumerator get.
            
            ifnot(theEnumerator next)
                [ ^ false ].
            
            theBeforePrev := theEnumerator get.
    
            ifnot (theEnumerator next)
                [ ^ false ].
            
            thePrevious := theEnumerator get.
        ].
                
        while (theEnumerator next)
        [
            if (theFilter(theBefore3Prev,theBefore2Prev,theBeforePrev,thePrevious,theEnumerator get))
                [ ^ true ].
            
            theBefore3Prev := theBefore2Prev.
            theBefore2Prev := theBeforePrev.
            theBeforePrev := thePrevious.
            thePrevious := theEnumerator get
        ].
        
        ^ false
    ]
    
    enumerable => theEnumerator.
}

// --- PatternFilter6 ---

public class PatternFilter6 :: Enumerator
{
    T<Func6>      theFilter.
    T<Enumerator> theEnumerator.
    object        thePrevious.
    object        theBeforePrev.
    object        theBefore2Prev.
    object        theBefore3Prev.
    object        theBefore4Prev.
    
    constructor new(Func6 aFilter, Enumerator anEnumerator)
    [
        theFilter := aFilter.
        theEnumerator := anEnumerator.
    ]
    
    get = theEnumerator get.
    
    reset
    [
        theEnumerator reset.
        thePrevious := nil.
    ]
    
    bool next
    [
        if (nil == thePrevious)
        [
            ifnot(theEnumerator next)
                [ ^ false ].
            
            theBefore4Prev := theEnumerator get.
            
            ifnot(theEnumerator next)
                [ ^ false ].
            
            theBefore3Prev := theEnumerator get.
            
            ifnot(theEnumerator next)
                [ ^ false ].
            
            theBefore2Prev := theEnumerator get.
            
            ifnot(theEnumerator next)
                [ ^ false ].
            
            theBeforePrev := theEnumerator get.
    
            ifnot(theEnumerator next)
                [ ^ false ].
            
            thePrevious := theEnumerator get
        ].
                                
        while (theEnumerator next)
        [
            if (theFilter(theBefore4Prev,theBefore3Prev,theBefore2Prev,theBeforePrev,thePrevious,theEnumerator get))
                [ ^ true ].
            
            theBefore4Prev := theBefore3Prev.
            theBefore3Prev := theBefore2Prev.
            theBefore2Prev := theBeforePrev.
            theBeforePrev := thePrevious.
            thePrevious := theEnumerator get
        ].
        
        ^ false
    ]
    
    enumerable => theEnumerator.
}

// --- Selecting ---

class Selector :: Enumerator
{
    T<Func1>      theSelector.
    T<Enumerator> theEnumerator.

    constructor new(Func1 aSelector, Enumerator anEnumerator)
    [
        theSelector := aSelector.
        theEnumerator := anEnumerator.
    ]
    
    constructor new(ExtensionMessage anExtension, Enumerator anEnumerator)
    [
        theSelector := (:x)[ ^ x~anExtension eval ].
        theEnumerator := anEnumerator.
    ]
    
    get = theSelector(theEnumerator get).
    
    bool next = theEnumerator next.
    
    reset 
    [
        theEnumerator reset.
    ]    
    
    enumerable => theEnumerator.
}

// --- Repeater ---

public class Repeater :: Enumerator
{
    int      theMin.
    int      theMax.
    int      theCurrent.
    T<Func1> theFunc.

    constructor new(int minValue, int maxValue, Func1 aRepetition)
    [
        theFunc := aRepetition.
        theMin := minValue.
        theMax := maxValue.
        theCurrent := theMin - 1.
    ]
    
    constructor new(int minValue, int maxValue, ExtensionMessage anExtension)
    [
        theFunc := (:x)[ ^ x~anExtension eval ].
        theMin := minValue.
        theMax := maxValue.
        theCurrent := theMin - 1.
    ]


    get = theFunc(theCurrent).
    
    reset
    [
        theCurrent := theMin - 1.
    ]
    
    bool next
    [
        theCurrent := theCurrent + 1.
        
        ^ (theCurrent < theMax).
    ]
    
    enumerable = Range from:theMin to:theMax.
}

//// --- ZipEnumerator ---
//
//class ZipEnumerator :: Enumerator
//{
//    enumerator theEnumerator1.
//    enumerator theEnumerator2.
//    
//    func2      theZipper.
//    
//    constructor new(BaseFunction2 fn, Enumerator enum1, Enumerator enum2)
//    [
//        theZipper := fn.
//        
//        theEnumerator1 := enum1.
//        theEnumerator2 := enum2.
//    ]
//    
//    constructor new(ExtensionMessage ext, Enumerator enum1, Enumerator enum2)
//        <= new func2:(:n:m)(n~ext eval:m) enumerator:enum1 enumerator:enum2.
//    
//    get = theZipper(theEnumerator1 get, theEnumerator2 get).
//    
//    bool next = theEnumerator1 next && theEnumerator2 next.
//    
//    reset
//    [
//        theEnumerator1 reset.
//        theEnumerator2 reset.
//    ]
//}

// --- RangeEnumerator ---

public class RangeEnumerator :: Enumerator
{
    object theVariable.
    object theStart.
    object theEnd.
    object theStep.
    
    bool   theForward.
    
    constructor for:aVariable from:aStart to:anEnd by:aStep
    [
        theVariable := aVariable.
        theStart := aStart.
        theEnd := anEnd.
        theStep := aStep.
        
        theForward := (aStart < anEnd).
        
        self reset.
    ]

    constructor from:aStart to:anEnd by:aStep
        <= for:(Variable new) from:aStart to:anEnd by:aStep.

    constructor from:aStart to:anEnd
        <= for:(Variable new) from:aStart to:anEnd by:1.
        
    reset 
    [
        theVariable value := theStart.

        if(theForward)
            [ theVariable reduce:theStep ];
            [ theVariable append:theStep ]
    ]    
    
    get = theVariable value.
    
    bool next
    [
        if(theForward)
            [ theVariable append:theStep. ^ (theVariable <= theEnd) ].
        
        theVariable reduce:theStep. 
        
        ^ (theVariable >= theEnd)
    ]
    
    enumerable = theVariable.
}

// --- BaseIndexer ---

class IntRangeEnumerator :: Enumerator<int> 
{
    int theIndex.
    int theStart.
    int theEnd.

    stacksafe constructor new(IntNumber start, IntNumber end)
    [
        theStart := start.
        theEnd := end.
        
        self reset.
    ]

    int get = theIndex.
            
    /// Returns false if the index out of range
    bool next 
    [
        theIndex += 1.
        
        ^ theIndex < theEnd
    ]
    
    /// Sets the i-index by specified value
    reset
    [ 
        theIndex := theStart - 1.
    ] 
    
    cast T<Enumerator> = Enumerator::{ embeddable dispatch => self. }.
    
    enumerable = Range from:theStart till:theEnd.
}

// --- Range --

public sealed class Range :: object, interface<Enumerable<int>>, interface<Enumerable>
{
    int theStart.
    int theEnd.
    
    constructor new(IntNumber start, IntNumber count)
    [
        theStart := start.
        theEnd := start + count.
    ]
    
    constructor from(IntNumber start) till(IntNumber end)
    [
        theStart := start.
        theEnd := end.
    ]
    
    constructor from(IntNumber start) to(IntNumber end)
    [
        theStart := start.
        theEnd := end + 1.
    ]
    
    stacksafe readLengthTo(ref<int> retVal)
    [
        int l := theEnd - theStart.
        
        retVal value := l.
    ]
    
    int rvprop length :: readLengthTo.
    
    Enumerator<int> enumerator = IntRangeEnumerator new(theStart, theEnd).
}

// --- IntervalEnumerator ---

public class IntervalEnumerator :: Enumerator
{
    object theVariable.
    object theStart.
    object theEnd.
    object theStep.
    
    bool   theForward.
    
    constructor for:aVariable from:aStart till:anEnd by:aStep
    [
        theVariable := aVariable.
        theStart := aStart.
        theEnd := anEnd.
        theStep := aStep.
        
        theForward := (aStart < anEnd).
        
        self reset.
    ]

    constructor from:aStart till:anEnd by:aStep
        <= for:(Variable new) from:aStart till:anEnd by:aStep.

    constructor from:aStart till:anEnd
        <= for:(Variable new) from:aStart till:anEnd by:1.
        
    reset 
    [
        theVariable value := theStart.

        if(theForward)
            [ theVariable reduce:theStep ];
            [ theVariable append:theStep ]
    ]    
    
    get = theVariable value.
    
    bool next
    [
        if(theForward )
            [ theVariable append:theStep. ^ (theVariable < theEnd) ].
        
        theVariable reduce:theStep. 
        
        ^ (theVariable > theEnd)
    ]
    
    enumerable = theVariable.
}

//// --- GroupingEnumerator ---
//
//class GroupingEnumerator :: Enumerator
//{
//    dictionary theDictionary.
//    enumerator theEnumerator.
//    func1      theGroupFunc.    
//    enumerator theGroupEnumerator.
//    
//    constructor new func1:grouper enumerator:enumerator
//    [
//        theGroupFunc := grouper.
//        theDictionary := Dictionary new.
//        theEnumerator := enumerator.
//        
//        $self reset.
//    ]
//    
//    constructor new ext_mssg:fn enumerator:enumerator
//        <= new func1:(:x)(x~fn eval) enumerator:enumerator.
//    
//    reset
//    [
//        theEnumerator reset.
//        
//        while (theEnumerator next)
//        [
//            var item := theEnumerator get.
//            var key := theGroupFunc(item).
//            
//            var groupItem := theDictionary[key].
//            if ($nil == groupItem)
//            [
//                groupItem := ArrayList new.
//                
//                theDictionary[key] := groupItem.
//            ].
//            
//            groupItem append:item.
//        ].
//        
//        theGroupEnumerator := theDictionary enumerator.
//    ]
//    
//    get = theGroupEnumerator get.
//    
//    bool next
//        = theGroupEnumerator next.
//}
//
//// --- OrderingEnumerator ---
//
//class OrderingEnumerator :: Enumerator
//{
//    enumerator theEnumerator.
//    func2      theOrderFunc.    
//    enumerator theListEnumerator.
//    
//    constructor new func2:orderFunc enumerator:enumerator
//    [
//        theOrderFunc := orderFunc.
//        theEnumerator := enumerator.
//        
//        $self reset.
//    ]
//    
//    constructor new func1:orderVal enumerator:enumerator
//    [
//        theOrderFunc := (:p:n)(orderVal(p) < orderVal(n) ).
//        theEnumerator := enumerator.
//        
//        $self reset.
//    ]
//    
//    constructor new ext_mssg:fn enumerator:enumerator
//        <= new func2:(:x:y)(x~fn eval:y) enumerator:enumerator.
//    
//    reset
//    [
//        theEnumerator reset.
//        
//        array list := Summing new:(ArrayList new); run enumerator:theEnumerator.
//        list sort func2:theOrderFunc.
//        
//        theListEnumerator := list enumerator.
//    ]
//    
//    get = theListEnumerator get.
//    
//    bool next
//        = theListEnumerator next.
//}

// === patterns ===

// --- BasePattern ---

public abstract class BasePattern :: Func1
{
    object theResult.
    
    run(Enumerator anEnumerator)
    [
        super run(anEnumerator).
        
        ^ theResult
    ]
}

// --- Summing ---

public class Summing :: BasePattern
{
    constructor new : aVariable
    [
        theResult := aVariable.
    ]

    closure eval : aParam [ theResult append:aParam. ]
}

// --- Accumulating ---

class Accumulating :: BasePattern
{
    T<Func2> theOperation.
    
    constructor new(object aVariable, Func2 anOperation)
    [
        theResult := aVariable.
        theOperation := anOperation.
    ]

    constructor new(object aVariable, ExtensionMessage anOperation)
    [
        theResult := aVariable.
        theOperation := (:a:b)[ a~anOperation eval:b ].
    ]

    closure eval : aParam [ theResult value := theOperation(theResult value,aParam). ]
}

//// --- patternOp ---
//
//extension patternOp
//{
//    skipping:aCounter
//        = SkipFilter new int:aCounter enumerator:(self enumerator).
//
//    last:counter
//        = TopFilter new int:counter int(self length - counter) enumerator:(self enumerator).
//
//    zip:anEnumerable by:aZipper
//        = ZipEnumerator new(aZipper, self enumerator, anEnumerable enumerator).
//
//    zip:anEnumerable forEach:func2
//        = self~patternOp zip:anEnumerable by:func2; forEach:idleFunc1.
//
//    groupBy:groupFunc
//        = GroupingEnumerator new(groupFunc, self enumerator).                                       
//
//    orderBy:orderFunc
//        = OrderingEnumerator new(orderFunc, self enumerator).
//
//    filter pattern:patternFun
//        = Filter new(patternFun, self enumerator).
//
//}