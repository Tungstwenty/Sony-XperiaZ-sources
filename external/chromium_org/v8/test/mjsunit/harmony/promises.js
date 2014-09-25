// Copyright 2013 the V8 project authors. All rights reserved.
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
//     * Redistributions of source code must retain the above copyright
//       notice, this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above
//       copyright notice, this list of conditions and the following
//       disclaimer in the documentation and/or other materials provided
//       with the distribution.
//     * Neither the name of Google Inc. nor the names of its
//       contributors may be used to endorse or promote products derived
//       from this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

// Flags: --harmony-promises --harmony-observation --allow-natives-syntax

var asyncAssertsExpected = 0;

function assertAsyncRan() { ++asyncAssertsExpected }

function assertAsync(b, s) {
  if (b) {
    print(s, "succeeded")
  } else {
    %AbortJS(s + " FAILED!")  // Simply throwing here will have no effect.
  }
  --asyncAssertsExpected
}

function assertAsyncDone(iteration) {
  var iteration = iteration || 0
  var dummy = {}
  Object.observe(dummy,
    function() {
      if (asyncAssertsExpected === 0)
        assertAsync(true, "all")
      else if (iteration > 10)  // Shouldn't take more.
        assertAsync(false, "all")
      else
        assertAsyncDone(iteration + 1)
    }
  )
  dummy.dummy = dummy
}


(function() {
  assertThrows(function() { new Promise(5) }, TypeError)
})();

(function() {
  assertThrows(function() { new Promise(function() { throw 5 }) }, 5)
})();

(function() {
  Promise.resolved(5);
  Promise.resolved(5).chain(undefined, assertUnreachable).chain(
    function(x) { assertAsync(x === 5, "resolved/chain-nohandler") },
    assertUnreachable
  )
  assertAsyncRan()
})();

(function() {
  Promise.rejected(5).chain(assertUnreachable, undefined).chain(
    assertUnreachable,
    function(r) { assertAsync(r === 5, "rejected/chain-nohandler") }
  )
  assertAsyncRan()
})();

(function() {
  Promise.resolved(5).then(undefined, assertUnreachable).chain(
    function(x) { assertAsync(x === 5, "resolved/then-nohandler") },
    assertUnreachable
  )
  assertAsyncRan()
})();

(function() {
  Promise.rejected(5).then(assertUnreachable, undefined).chain(
    assertUnreachable,
    function(r) { assertAsync(r === 5, "rejected/then-nohandler") }
  )
  assertAsyncRan()
})();

(function() {
  var p1 = Promise.resolved(5)
  var p2 = Promise.resolved(p1)
  var p3 = Promise.resolved(p2)
  p3.chain(
    function(x) { assertAsync(x === p2, "resolved/chain") },
    assertUnreachable
  )
  assertAsyncRan()
})();

(function() {
  var p1 = Promise.resolved(5)
  var p2 = Promise.resolved(p1)
  var p3 = Promise.resolved(p2)
  p3.then(
    function(x) { assertAsync(x === 5, "resolved/then") },
    assertUnreachable
  )
  assertAsyncRan()
})();

(function() {
  var p1 = Promise.rejected(5)
  var p2 = Promise.resolved(p1)
  var p3 = Promise.resolved(p2)
  p3.chain(
    function(x) { assertAsync(x === p2, "rejected/chain") },
    assertUnreachable
  )
  assertAsyncRan()
})();

(function() {
  var p1 = Promise.rejected(5)
  var p2 = Promise.resolved(p1)
  var p3 = Promise.resolved(p2)
  p3.then(
    assertUnreachable,
    function(x) { assertAsync(x === 5, "rejected/then") }
  )
  assertAsyncRan()
})();

(function() {
  var p1 = Promise.resolved(5)
  var p2 = Promise.resolved(p1)
  var p3 = Promise.resolved(p2)
  p3.chain(function(x) { return x }, assertUnreachable).chain(
    function(x) { assertAsync(x === p1, "resolved/chain/chain") },
    assertUnreachable
  )
  assertAsyncRan()
})();

(function() {
  var p1 = Promise.resolved(5)
  var p2 = Promise.resolved(p1)
  var p3 = Promise.resolved(p2)
  p3.chain(function(x) { return x }, assertUnreachable).then(
    function(x) { assertAsync(x === 5, "resolved/chain/then") },
    assertUnreachable
  )
  assertAsyncRan()
})();

(function() {
  var p1 = Promise.resolved(5)
  var p2 = Promise.resolved(p1)
  var p3 = Promise.resolved(p2)
  p3.chain(function(x) { return 6 }, assertUnreachable).chain(
    function(x) { assertAsync(x === 6, "resolved/chain/chain2") },
    assertUnreachable
  )
  assertAsyncRan()
})();

(function() {
  var p1 = Promise.resolved(5)
  var p2 = Promise.resolved(p1)
  var p3 = Promise.resolved(p2)
  p3.chain(function(x) { return 6 }, assertUnreachable).then(
    function(x) { assertAsync(x === 6, "resolved/chain/then2") },
    assertUnreachable
  )
  assertAsyncRan()
})();

(function() {
  var p1 = Promise.resolved(5)
  var p2 = Promise.resolved(p1)
  var p3 = Promise.resolved(p2)
  p3.then(function(x) { return x + 1 }, assertUnreachable).chain(
    function(x) { assertAsync(x === 6, "resolved/then/chain") },
    assertUnreachable
  )
  assertAsyncRan()
})();

(function() {
  var p1 = Promise.resolved(5)
  var p2 = Promise.resolved(p1)
  var p3 = Promise.resolved(p2)
  p3.then(function(x) { return x + 1 }, assertUnreachable).then(
    function(x) { assertAsync(x === 6, "resolved/then/then") },
    assertUnreachable
  )
  assertAsyncRan()
})();

(function() {
  var p1 = Promise.resolved(5)
  var p2 = Promise.resolved(p1)
  var p3 = Promise.resolved(p2)
  p3.then(function(x){ return Promise.resolved(x+1) }, assertUnreachable).chain(
    function(x) { assertAsync(x === 6, "resolved/then/chain2") },
    assertUnreachable
  )
  assertAsyncRan()
})();

(function() {
  var p1 = Promise.resolved(5)
  var p2 = Promise.resolved(p1)
  var p3 = Promise.resolved(p2)
  p3.then(function(x) { return Promise.resolved(x+1) }, assertUnreachable).then(
    function(x) { assertAsync(x === 6, "resolved/then/then2") },
    assertUnreachable
  )
  assertAsyncRan()
})();

(function() {
  var p1 = Promise.resolved(5)
  var p2 = Promise.resolved(p1)
  var p3 = Promise.resolved(p2)
  p3.chain(function(x) { throw 6 }, assertUnreachable).chain(
    assertUnreachable,
    function(x) { assertAsync(x === 6, "resolved/chain-throw/chain") }
  )
  assertAsyncRan()
})();

(function() {
  var p1 = Promise.resolved(5)
  var p2 = Promise.resolved(p1)
  var p3 = Promise.resolved(p2)
  p3.chain(function(x) { throw 6 }, assertUnreachable).then(
    assertUnreachable,
    function(x) { assertAsync(x === 6, "resolved/chain-throw/then") }
  )
  assertAsyncRan()
})();

(function() {
  var p1 = Promise.resolved(5)
  var p2 = Promise.resolved(p1)
  var p3 = Promise.resolved(p2)
  p3.then(function(x) { throw 6 }, assertUnreachable).chain(
    assertUnreachable,
    function(x) { assertAsync(x === 6, "resolved/then-throw/chain") }
  )
  assertAsyncRan()
})();

(function() {
  var p1 = Promise.resolved(5)
  var p2 = Promise.resolved(p1)
  var p3 = Promise.resolved(p2)
  p3.then(function(x) { throw 6 }, assertUnreachable).then(
    assertUnreachable,
    function(x) { assertAsync(x === 6, "resolved/then-throw/then") }
  )
  assertAsyncRan()
})();

(function() {
  var p1 = Promise.resolved(5)
  var p2 = {then: function(onResolve, onReject) { onResolve(p1) }}
  var p3 = Promise.resolved(p2)
  p3.chain(
    function(x) { assertAsync(x === p2, "resolved/thenable/chain") },
    assertUnreachable
  )
  assertAsyncRan()
})();

(function() {
  var p1 = Promise.resolved(5)
  var p2 = {then: function(onResolve, onReject) { onResolve(p1) }}
  var p3 = Promise.resolved(p2)
  p3.then(
    function(x) { assertAsync(x === 5, "resolved/thenable/then") },
    assertUnreachable
  )
  assertAsyncRan()
})();

(function() {
  var p1 = Promise.rejected(5)
  var p2 = {then: function(onResolve, onReject) { onResolve(p1) }}
  var p3 = Promise.resolved(p2)
  p3.chain(
    function(x) { assertAsync(x === p2, "rejected/thenable/chain") },
    assertUnreachable
  )
  assertAsyncRan()
})();

(function() {
  var p1 = Promise.rejected(5)
  var p2 = {then: function(onResolve, onReject) { onResolve(p1) }}
  var p3 = Promise.resolved(p2)
  p3.then(
    assertUnreachable,
    function(x) { assertAsync(x === 5, "rejected/thenable/then") }
  )
  assertAsyncRan()
})();

(function() {
  var deferred = Promise.deferred()
  var p1 = deferred.promise
  var p2 = Promise.resolved(p1)
  var p3 = Promise.resolved(p2)
  p3.chain(
    function(x) { assertAsync(x === p2, "chain/resolve") },
    assertUnreachable
  )
  deferred.resolve(5)
  assertAsyncRan()
})();

(function() {
  var deferred = Promise.deferred()
  var p1 = deferred.promise
  var p2 = Promise.resolved(p1)
  var p3 = Promise.resolved(p2)
  p3.then(
    function(x) { assertAsync(x === 5, "then/resolve") },
    assertUnreachable
  )
  deferred.resolve(5)
  assertAsyncRan()
})();

(function() {
  var deferred = Promise.deferred()
  var p1 = deferred.promise
  var p2 = Promise.resolved(p1)
  var p3 = Promise.resolved(p2)
  p3.chain(
    function(x) { assertAsync(x === p2, "chain/reject") },
    assertUnreachable
  )
  deferred.reject(5)
  assertAsyncRan()
})();

(function() {
  var deferred = Promise.deferred()
  var p1 = deferred.promise
  var p2 = Promise.resolved(p1)
  var p3 = Promise.resolved(p2)
  p3.then(
    assertUnreachable,
    function(x) { assertAsync(x === 5, "then/reject") }
  )
  deferred.reject(5)
  assertAsyncRan()
})();

(function() {
  var deferred = Promise.deferred()
  var p1 = deferred.promise
  var p2 = {then: function(onResolve, onReject) { onResolve(p1) }}
  var p3 = Promise.resolved(p2)
  p3.chain(
    function(x) { assertAsync(x === p2, "chain/resolve/thenable") },
    assertUnreachable
  )
  deferred.resolve(5)
  assertAsyncRan()
})();

(function() {
  var deferred = Promise.deferred()
  var p1 = deferred.promise
  var p2 = {then: function(onResolve, onReject) { onResolve(p1) }}
  var p3 = Promise.resolved(p2)
  p3.then(
    function(x) { assertAsync(x === 5, "then/resolve/thenable") },
    assertUnreachable
  )
  deferred.resolve(5)
  assertAsyncRan()
})();

(function() {
  var deferred = Promise.deferred()
  var p1 = deferred.promise
  var p2 = {then: function(onResolve, onReject) { onResolve(p1) }}
  var p3 = Promise.resolved(p2)
  p3.chain(
    function(x) { assertAsync(x === p2, "chain/reject/thenable") },
    assertUnreachable
  )
  deferred.reject(5)
  assertAsyncRan()
})();

(function() {
  var deferred = Promise.deferred()
  var p1 = deferred.promise
  var p2 = {then: function(onResolve, onReject) { onResolve(p1) }}
  var p3 = Promise.resolved(p2)
  p3.then(
    assertUnreachable,
    function(x) { assertAsync(x === 5, "then/reject/thenable") }
  )
  deferred.reject(5)
  assertAsyncRan()
})();

(function() {
  var p1 = Promise.resolved(5)
  var p2 = Promise.resolved(p1)
  var deferred = Promise.deferred()
  var p3 = deferred.promise
  p3.chain(
    function(x) { assertAsync(x === p2, "chain/resolve2") },
    assertUnreachable
  )
  deferred.resolve(p2)
  assertAsyncRan()
})();

(function() {
  var p1 = Promise.resolved(5)
  var p2 = Promise.resolved(p1)
  var deferred = Promise.deferred()
  var p3 = deferred.promise
  p3.then(
    function(x) { assertAsync(x === 5, "then/resolve2") },
    assertUnreachable
  )
  deferred.resolve(p2)
  assertAsyncRan()
})();

(function() {
  var p1 = Promise.resolved(5)
  var p2 = Promise.resolved(p1)
  var deferred = Promise.deferred()
  var p3 = deferred.promise
  p3.chain(
    assertUnreachable,
    function(x) { assertAsync(x === 5, "chain/reject2") }
  )
  deferred.reject(5)
  assertAsyncRan()
})();

(function() {
  var p1 = Promise.resolved(5)
  var p2 = Promise.resolved(p1)
  var deferred = Promise.deferred()
  var p3 = deferred.promise
  p3.then(
    assertUnreachable,
    function(x) { assertAsync(x === 5, "then/reject2") }
  )
  deferred.reject(5)
  assertAsyncRan()
})();

(function() {
  var p1 = Promise.resolved(5)
  var p2 = {then: function(onResolve, onReject) { onResolve(p1) }}
  var deferred = Promise.deferred()
  var p3 = deferred.promise
  p3.chain(
    function(x) { assertAsync(x === p2, "chain/resolve/thenable2") },
    assertUnreachable
  )
  deferred.resolve(p2)
  assertAsyncRan()
})();

(function() {
  var p1 = Promise.resolved(5)
  var p2 = {then: function(onResolve, onReject) { onResolve(p1) }}
  var deferred = Promise.deferred()
  var p3 = deferred.promise
  p3.then(
    function(x) { assertAsync(x === 5, "then/resolve/thenable2") },
    assertUnreachable
  )
  deferred.resolve(p2)
  assertAsyncRan()
})();

(function() {
  var p1 = Promise.resolved(0)
  var p2 = p1.chain(function(x) { return p2 }, assertUnreachable)
  p2.chain(
    assertUnreachable,
    function(r) { assertAsync(r instanceof TypeError, "cyclic/chain") }
  )
  assertAsyncRan()
})();

(function() {
  var p1 = Promise.resolved(0)
  var p2 = p1.then(function(x) { return p2 }, assertUnreachable)
  p2.chain(
    assertUnreachable,
    function(r) { assertAsync(r instanceof TypeError, "cyclic/then") }
  )
  assertAsyncRan()
})();

(function() {
  var deferred = Promise.deferred()
  var p = deferred.promise
  deferred.resolve(p)
  p.chain(
    function(x) { assertAsync(x === p, "cyclic/deferred/chain") },
    assertUnreachable
  )
  assertAsyncRan()
})();

(function() {
  var deferred = Promise.deferred()
  var p = deferred.promise
  deferred.resolve(p)
  p.then(
    assertUnreachable,
    function(r) { assertAsync(r instanceof TypeError, "cyclic/deferred/then") }
  )
  assertAsyncRan()
})();

(function() {
  Promise.all([]).chain(
    function(x) { assertAsync(x.length === 0, "all/resolve/empty") },
    assertUnreachable
  )
  assertAsyncRan()
})();

(function() {
  var deferred1 = Promise.deferred()
  var p1 = deferred1.promise
  var deferred2 = Promise.deferred()
  var p2 = deferred2.promise
  var deferred3 = Promise.deferred()
  var p3 = deferred3.promise
  Promise.all([p1, p2, p3]).chain(
    function(x) {
      assertAsync(x.length === 3, "all/resolve")
      assertAsync(x[0] === 1, "all/resolve/0")
      assertAsync(x[1] === 2, "all/resolve/1")
      assertAsync(x[2] === 3, "all/resolve/2")
    },
    assertUnreachable
  )
  deferred1.resolve(1)
  deferred3.resolve(3)
  deferred2.resolve(2)
  assertAsyncRan()
  assertAsyncRan()
  assertAsyncRan()
  assertAsyncRan()
})();

(function() {
  var deferred = Promise.deferred()
  var p1 = deferred.promise
  var p2 = Promise.resolved(2)
  var p3 = Promise.deferred().promise
  Promise.all([p1, p2, p3]).chain(
    assertUnreachable,
    assertUnreachable
  )
  deferred.resolve(1)
})();

(function() {
  var deferred1 = Promise.deferred()
  var p1 = deferred1.promise
  var deferred2 = Promise.deferred()
  var p2 = deferred2.promise
  var deferred3 = Promise.deferred()
  var p3 = deferred3.promise
  Promise.all([p1, p2, p3]).chain(
    assertUnreachable,
    function(x) { assertAsync(x === 2, "all/reject") }
  )
  deferred1.resolve(1)
  deferred3.resolve(3)
  deferred2.reject(2)
  assertAsyncRan()
})();

(function() {
  Promise.one([]).chain(
    assertUnreachable,
    assertUnreachable
  )
})();

(function() {
  var p1 = Promise.resolved(1)
  var p2 = Promise.resolved(2)
  var p3 = Promise.resolved(3)
  Promise.one([p1, p2, p3]).chain(
    function(x) { assertAsync(x === 1, "resolved/all") },
    assertUnreachable
  )
  assertAsyncRan()
})();

(function() {
  var p1 = Promise.resolved(1)
  var p2 = Promise.resolved(2)
  var p3 = Promise.resolved(3)
  Promise.one([0, p1, p2, p3]).chain(
    function(x) { assertAsync(x === 0, "resolved-const/all") },
    assertUnreachable
  )
  assertAsyncRan()
})();

(function() {
  var deferred1 = Promise.deferred()
  var p1 = deferred1.promise
  var deferred2 = Promise.deferred()
  var p2 = deferred2.promise
  var deferred3 = Promise.deferred()
  var p3 = deferred3.promise
  Promise.one([p1, p2, p3]).chain(
    function(x) { assertAsync(x === 3, "one/resolve") },
    assertUnreachable
  )
  deferred3.resolve(3)
  deferred1.resolve(1)
  assertAsyncRan()
})();

(function() {
  var deferred = Promise.deferred()
  var p1 = deferred.promise
  var p2 = Promise.resolved(2)
  var p3 = Promise.deferred().promise
  Promise.one([p1, p2, p3]).chain(
    function(x) { assertAsync(x === 2, "resolved/one") },
    assertUnreachable
  )
  deferred.resolve(1)
  assertAsyncRan()
})();

(function() {
  var deferred1 = Promise.deferred()
  var p1 = deferred1.promise
  var deferred2 = Promise.deferred()
  var p2 = deferred2.promise
  var deferred3 = Promise.deferred()
  var p3 = deferred3.promise
  Promise.one([p1, p2, p3]).chain(
    function(x) { assertAsync(x === 3, "one/resolve/reject") },
    assertUnreachable
  )
  deferred3.resolve(3)
  deferred1.reject(1)
  assertAsyncRan()
})();

(function() {
  var deferred1 = Promise.deferred()
  var p1 = deferred1.promise
  var deferred2 = Promise.deferred()
  var p2 = deferred2.promise
  var deferred3 = Promise.deferred()
  var p3 = deferred3.promise
  Promise.one([p1, p2, p3]).chain(
    assertUnreachable,
    function(x) { assertAsync(x === 3, "one/reject/resolve") }
  )
  deferred3.reject(3)
  deferred1.resolve(1)
  assertAsyncRan()
})();

(function() {
  var log
  function MyPromise(resolver) {
    log += "n"
    Promise.call(this,
      function(resolve, reject) {
        resolver(
          function(x) { log += "x" + x; resolve(x) },
          function(r) { log += "r" + r; reject(r) }
        )
      }
    )
  }

  MyPromise.__proto__ = Promise
  MyPromise.deferred = function() {
    log += "d"
    return this.__proto__.deferred.call(this)
  }

  MyPromise.prototype.__proto__ = Promise.prototype
  MyPromise.prototype.chain = function(resolve, reject) {
    log += "w"
    return this.__proto__.__proto__.chain.call(this, resolve, reject)
  }

  log = ""
  var p1 = new MyPromise(function(resolve, reject) { resolve(1) })
  var p2 = new MyPromise(function(resolve, reject) { reject(2) })
  var d3 = MyPromise.deferred()
  assertTrue(d3.promise instanceof MyPromise, "subclass/instance3")
  assertTrue(log === "nx1nr2dn", "subclass/create")

  log = ""
  var p4 = MyPromise.resolved(4)
  var p5 = MyPromise.rejected(5)
  assertTrue(p4 instanceof MyPromise, "subclass/instance4")
  assertTrue(p5 instanceof MyPromise, "subclass/instance5")
  d3.resolve(3)
  assertTrue(log === "nx4nr5x3", "subclass/resolve")

  log = ""
  var d6 = MyPromise.deferred()
  d6.promise.chain(function(x) { return new Promise(x) }).chain(function() {})
  d6.resolve(6)
  assertTrue(log === "dnwnwnx6", "subclass/chain")

  log = ""
  Promise.all([11, Promise.resolved(12), 13, MyPromise.resolved(14), 15, 16])
  assertTrue(log === "nx14wn", "subclass/all/arg")

  log = ""
  MyPromise.all([21, Promise.resolved(22), 23, MyPromise.resolved(24), 25, 26])
  assertTrue(log === "nx24dnnx21wnnx23wnwnnx25wnnx26wn", "subclass/all/self")
})();


assertAsyncDone()
