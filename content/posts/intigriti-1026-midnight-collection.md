---
id: "intigriti-1026-midnight-collection"
title: "The Midnight Collection"
competition: "Intigriti October Challenge 2026"
competitionLogo: "/media/intigriti-logo.webp"
category: "CTF Writeups"
date: "2026-10-23"
summary: "A Halloween candy shop sells one €666 box and hands you €66 of credit. Two coupons stack to 50% and no further, which looks like a wall until the duplicate-use guard turns out to only check the coupon you added last."
flag: "INTIGRITI{094638e6-ba2f-4c50-8bb4-41ced0d5debd}"
tags:
  - "Web Security"
  - "Business Logic"
---

## Recon

It's a Halloween candy shop, and it's a lovely piece of set dressing. Soft pink gummies, a jar of
striped humbugs, a banner counting down to midnight. Then there's the thing in the corner you're
clearly meant to want: The Midnight Collection, a numbered black chest with an orange ribbon,
€666.00, badged "Only one available".

The shop gives you €66.00 of store credit.

So the whole puzzle is sitting in the price tag before you touch anything. You need 90% off, the
shop advertises 50%, and the flag is inside the chest.

The front end talks to a small JSON API, and it's refreshingly small:

```
GET  /api/session        issues a session cookie and €66.00 of credit
GET  /api/products       six products
POST /api/cart/items     {"productId": "..."}
POST /api/cart/coupon    {"code": "..."}
POST /api/cart/coupon/remove
POST /api/newsletter     {"email": "..."}   hands you a promotion code
POST /api/checkout       {}                 returns a receipt
```

Six products. Five sweets between €8.90 and €14.90, and the chest at €666.00. `/health` answers
`{"ok":true,"service":"ble"}`, which is where the `BLE-` on every order ID comes from, and that's
the only introspection the app gives up.

Two promotion codes exist. NIGHT20 gives 20% and is printed in the banner on every page, so the
shop wants you to have it. NEWS30 gives 30% and arrives if you subscribe to the newsletter with
any address at all. Both refuse to apply unless the chest is already in your bag:

```json
{"error":"coupon_restricted","message":"Promotions are reserved for The Midnight Collection."}
```

Apply both and the discount stacks to 50%. The chest drops to €333.00.

![The bag with both promotions applied once each: subtotal €666.00, NIGHT20 and NEWS30 each listed once, total €333.00, "Still needed: €267.00"](/media/intigriti-1026/01-bag-50pct.webp)

You have €66.00. This is where the challenge actually starts.

I got there eventually. I also spent most of a day writing a proof that it was impossible, which
is the interesting part, so I'm keeping the wrong turns in.

## The arithmetic nobody wants to do

My instinct was to start trying things. I'm glad I did the boring thing instead, because it told
me which part of the shop was worth attacking.

Three behaviours, measured and written down. Each promotion stores its own `amount`, fixed at the
instant you apply it. The bag's `discount` is the sum of those amounts. The percentage comes off
the entire subtotal. And the promotion gate needs the chest in the bag, which means the subtotal
is never below €666.00 while any promotion exists.

Now the numbers box you in. The biggest bag in the shop is all six products, €722.70. Quantity is
hardcoded to 1 and you can't add the same product twice, so that's genuinely the ceiling. Half of
that is €361.35, and that's the largest discount two codes can produce. The cheapest possible
order containing the chest is therefore €333.00, against €66.00 of credit. Credit only ever falls,
because checkout subtracts a total that's always positive, so you can't farm it either.

That's a factor of five, not a rounding error. No clever ordering of two codes worth 50% closes
it.

Which gave me the one useful sentence of the whole hunt: **the solve requires a third promotion on
the bag**, and exactly one thing in the entire API can put one there. From that point it wasn't
"find a bug in the shop", it was "make `POST /api/cart/coupon` accept a code it means to refuse".

I then spent a day failing to do that.

## Dead end: getting something out of the bag

The frozen `amount` was the first thing that smelled wrong. A discount captured once and never
recalculated is a classic move: apply the coupon while the bag is expensive, then make the bag
cheap and keep the big discount. And it is genuinely frozen:

```
chest alone, subtotal 66600  ->  apply NIGHT20  ->  amount 13320
add a €8.90 bag of sweets    ->  subtotal 67490,  amount STILL 13320
```

A fresh NIGHT20 on that bag would book 13498. The stored figure doesn't move.

The shop practically dares you, too. On the bag page, in small grey text:

> Items are reserved once added and cannot be removed from this bag.

Nobody writes that sentence unless removal is interesting. I went after it hard. Every removal
route name I could think of under `/api/cart/`, with four different body shapes. `PUT`, `DELETE`
and `PATCH` on the item endpoint, which all came back `501`, because the server implements `GET`
and `POST` and nothing else. Quantity fields, in case I could set one to zero. A size cap that
might evict the oldest item if I added all six products.

Nothing. The sentence was just true, which I didn't expect.

Worse, when I went back to the arithmetic I realised the freeze can't help even in principle. The
subtotal only ever grows, so a frozen discount is always *smaller* than a fresh one. I'd been
chasing a behaviour that makes the attacker's position worse.

## Dead end: the same code twice

The obvious move. Apply NIGHT20, apply NIGHT20 again.

```json
{"error":"coupon_already_applied","message":"Coupon already applied."}
```

So there's a guard. Guards on string comparisons are usually soft, so I went at the string.
Lowercase, mixed case, leading and trailing spaces, a non-breaking space, the Turkish dotless i,
the long s, every Unicode form whose uppercase folds back to `NIGHT20`, full-width digits, and the
same set in the other order in case the stored value was the raw input rather than the canonical
one.

Every single one: 409.

Then I worked out why, which is the part worth taking away. Hand the endpoint a non-string `code`
and the backend throws an unhandled `AttributeError`, which tells you the handler normalises your
input once with something equivalent to `code.strip().upper()`, and then uses that same
normalised string for both the table lookup and the duplicate check. When one value feeds both
decisions, no string trick can separate them. You can't be valid and unseen at the same time.

So I crossed the whole class off. Correctly, as it turns out.

While I was in there I also swept roughly 400 themed coupon candidates, 50 hidden parameter names
on the coupon endpoint against three values each, and the usual trusted-header spoofs. Two codes
exist, the endpoint reads one field, and the server doesn't care who it thinks you are.

## Dead end: the race

Check-then-insert on shared state is exactly the shape that loses to concurrency, and by this
point I *wanted* it to be a race, which should have been a warning sign.

I did it properly. Not twenty threads and hope. An HTTP/2 single-packet attack: one TLS
connection, twenty-five streams, each sent as headers plus the request body minus its very last
byte, all of them parked at the server. Then every last byte flushed in a single write, so all
twenty-five requests complete inside one TCP segment and arrive with no jitter between them.

One `200`. Twenty-four `409`s.

I ran it three times. Same result every time, which is about as clean a negative as you get. The
backend serialises these requests and the window doesn't exist.

## The proof, and why it was wrong

At that point I had what looked like a rigorous proof, and it was wrong.

I could show the discount ceiling was €361.35 against a €666.00 chest. I could show the bag only
grows, that credit only falls, that no string bypasses the guard, that there's no race, that
nothing can leave the bag. I'd confirmed the flag needs the chest specifically by buying all five
cheap sweets (affordable at €56.70) and getting a receipt with no flag in it. I'd found the real
attack surface by feeding every input field a value of the wrong type and watching for the crash
that tells you the server actually reads it. Two fields. That's the whole shop.

So I wrote up the conclusion that the challenge couldn't be solved as I understood it, and that my
model must have a hole in it.

That last clause was doing a lot of work and I should have taken it more seriously. When your
reasoning says a published challenge is impossible, your reasoning is wrong. The only question is
where.

## The number that never changed

It was in front of me the entire time.

Every promotion in the cart JSON looks like this:

```json
{"code":"NIGHT20","percent":20,"amount":13320,"count":1}
```

I'd read that object hundreds of times. `count` is always 1. I logged it, I printed it in every
debug line, and I filed it mentally under "dead field from a template".

Then I read the shop's own JavaScript:

```js
`Promotion ${promotion.code} · ${promotion.percent}% off${promotion.count > 1 ? ` × ${promotion.count}` : ""}`
```

Somebody wrote a branch for `count > 1`. Somebody wrote the code that prints `× 2` on the bag
page. That branch exists because the shop can reach a state where a promotion has been applied
more than once.

A field that reads 1 in every response you've ever seen isn't dead code. It's a state you haven't
reached yet.

## X, Y, X

So I went back over my own testing and found the shape of the hole.

Every duplicate test I'd run was the same code twice in a row, or the same code after removing it.
`NIGHT20, NIGHT20`. `NIGHT20, remove, NIGHT20`. Dozens of variants, all of them those two shapes.

I had never once tried the same code after a **different** code.

```
NIGHT20, NIGHT20            -> 409 coupon_already_applied
NIGHT20, NEWS30, NIGHT20    -> 200 accepted
```

There it is. The guard compares your code against the promotion added **most recently**, not
against every promotion on the bag. Put the other code in between and the check looks at the wrong
entry and waves you straight through.

Alternate the two codes and each one applies twice.

![The same bag after the interleaved sequence: both promotions now read "× 2", subtotal still €666.00, total €0.00, Complete order button live](/media/intigriti-1026/02-bag-stacked-total0.webp)

## Why it works

Four separate decisions have to line up, and only one of them is the bug.

First, the guard inspects the wrong thing. It compares against the tail of the promotions list
instead of searching all of it. On its own, that's the whole vulnerability.

Second, a repeat merges into the existing promotion instead of creating a new one. When the guard
does let a repeat through, the handler doesn't append a second row. It finds the existing entry
and updates it in place, incrementing `count` and adding to `amount`:

```
third request:  NIGHT20  amount 13320 -> 26640,  count 1 -> 2
```

This is my favourite part. That code path was written on purpose. It's the reason `count` exists,
and the reason the interface already knows how to render `× 2`. Someone built stacking
deliberately and put a guard in front of it to keep it unreachable. The feature works perfectly;
the guard is what's broken. A merge-on-repeat design is only ever as safe as the check standing in
front of it.

Third, the discount accumulates instead of being recalculated. Each application adds to a stored
figure. Nothing recomputes the discount from the bag afterwards and nothing caps it at the
subtotal, so repeats compound rather than overwrite.

Fourth, the percentage covers the whole bag. The banner promises 20% off The Midnight Collection,
but the amount is taken from the full subtotal. With the chest plus one €8.90 bag of sweets,
NIGHT20 books €134.98, 20% of €674.90, not 20% of the chest. On its own that's a pricing bug worth
a few euro; combined with the other three it points the entire stacked discount at the full bag
value.

And the frozen amount, the thing I chased hardest on day one, turns out to be the one behaviour
here that plays no part in the exploit at all.

## The solve

Add the chest, subscribe to the newsletter to release NEWS30, then alternate:

```
{"code":"NIGHT20"}   discount  13320   total  53280   NIGHT20 count 1
{"code":"NEWS30"}    discount  33300   total  33300   NEWS30  count 1
{"code":"NIGHT20"}   discount  46620   total  19980   NIGHT20 count 2   <-- accepted again
{"code":"NEWS30"}    discount  66600   total      0   NEWS30  count 2
```

20 + 30 + 20 + 30 = 100% of the subtotal. `canCheckout` flips to true, and `POST /api/checkout`
opens the chest:

```json
{"ok":true,"receipt":{"orderId":"BLE-439A2DE822","paid":0,"credit":6600,
 "message":"Your order is confirmed and being prepared.",
 "flag":"INTIGRITI{094638e6-ba2f-4c50-8bb4-41ced0d5debd}"}}
```

A €666.00 chest, paid with nothing, and the €66.00 still sitting in the account afterwards.

## The punchline

The quick fix is one line: search the promotions list instead of looking at its last entry. The
real fix is to stop storing a discount at all. Recompute it from the bag on every change, cap it
at the subtotal, and apply each percentage only to the items the promotion actually covers.

But the thing I'll remember is that the shop was naming the bug in every single response it sent
me. `"count": 1`, hundreds of times, while I was busy proving the challenge was impossible.

## Afterword

Two things, and they cost me a day between them.

My duplicate testing covered `X, X` and `X, remove, X`, and on that evidence I declared the guard
sound and moved on. The interleaved case `X, Y, X` is the one that mattered and the one I never
ran. Any time an action is meant to happen once, the matrix is `X,X`, `X,Y,X`, `X,remove,X`,
`X,Y,X,Y`. It's four tests. I ran two.

And an API field that sits at 1 forever isn't nothing: the response body documents the state
machine behind it, including the parts of it the developer didn't mean to leave reachable.

Nice challenge. The impossible-looking arithmetic is doing real work here: it pushes you into
proving a negative instead of widening the test matrix by one case.
