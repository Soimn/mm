- would be handy if there was a way of descibing an array literal that *must contain exactly* N elements. This would allow catching bugs similar to incomplete switch statements.

- a shorter syntax for chained or, and chained and with repeated left hand side (a == 0 || a == 1 || ..) might be a useful addition. (a candidate would be an in operator like python, but this seems like adding way too much complexity for somthing relatively small)

- headers are useful for describing APIs, classes can be used to organize code into library like collections with compiler enforced visibility restrictions (public/private). Is there something that provides similar benefits with less drawbacks?