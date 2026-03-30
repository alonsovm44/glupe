JoeAltmaier 21 hours ago | unvote | next [–]

I wrote a programming language, some time back. You need a good reason to add to the tumult in the marketplace, and I thought I had one. My language was for discrete control systems. You could declare samplers for data values (interval, sensor, type) and name them as variables. You could create control actions by listing a set of one or more sampler variables in brackets. Once there were 'fresh' values for all of the samplers, the action would be invoked.
It had the usual functions and i/o library stuff. In fact I wrote a tool to absorb other library headers e.g. C or C++ and product blocks that my compiler could link with, and voila your program could call those external libraries.

We used it for a couple of contracts. Some of the control engineers were enthusiastic; some not so much. One more thing to learn.

reply

	
Chance-Device 21 hours ago | prev | next [–]

But… why not just write pseudo code, or any language you actually know, and just ask the AI to port it to the language you want? That’s a serious question by the way, is there some use case here I’m not seeing where learning this new syntax and running this actually helps instead of being extra steps nobody needs?
reply

	
Paracompact 19 hours ago | parent | next [–]

Indeed, it seems to occupy a middle ground between fast-and-easy AI prompting, and slow-but-robust traditional programming. But it's still relying on AI outputs unconstrained (as far as I can tell) by more formal methods and semantic checks.
But it's also hard for me to grasp the exact value add from the README, or why I should buy their story, so I'm not sure.

reply

	
janwirth 17 hours ago | root | parent | next [–]

Install my executable bro, trust me just one more tool and you will be the 10x engineer!!!
reply

	
mitul005 4 hours ago | prev | next [–]

The container model is the interesting bit, AI can only touch what's inside $$...$$ blocks, the rest is frozen. But I'm not sure "programming language" is the right frame for it; it's closer to annotated scaffolding that delegates to an LLM. The real question is whether the caching via container hashing is reliable enough that you're not re-spending tokens on identical blocks across runs.
reply

	
Retr0id 22 hours ago | prev | next [–]

I've been thinking about something along these lines, but coupled with deterministic inference. At each "macro" invocation you'd also include hash-of-model, and hash-of-generated-text. (Note, determinism doesn't require temperature 0, so long as you can control the rng seed. But there are a lot of other things that make determinism hard)
You could take it a step further and have a deterministic agent inside a deterministic VM, and you can share a whole project as {model hash, vm image hash, prompt, source tree hash} and have someone else deterministically reproduce it.

Is this useful? Not sure. One use case I had in mind as a mechanism for distributing "forbidden software". You can't distribute software that violates DMCA, for example, but can you distribute a prompt?

reply

	
mpalmer 21 hours ago | parent | next [–]

Deterministic inference is mechanically indistinguishable from decompression or decryption, so if there's a way to one-weird-trick DMCA, it's probably not this.
reply

	
bee_rider 21 hours ago | root | parent | next [–]

You’d think that, but it sees like big business and governments are treating inference as somehow special. I dunno, maybe low temperatures can highlight this weird situation?
Temperature is an easy knob to twist, after all. Somebody (not me I’m too poor to pay the lawyers) should do a search and find where the crime starts.

reply

	
Retr0id 21 hours ago | root | parent | next [–]

What does temperature have to do with anything?
reply

	
bee_rider 20 hours ago | root | parent | next [–]

Or however deterministic inference is supposed to happen. I don’t know LLMs.
reply

	
janwirth 17 hours ago | root | parent | next [–]

https://152334h.github.io/blog/non-determinism-in-gpt-4/
reply

	
mpalmer 21 hours ago | root | parent | prev | next [–]

Well, it's still not deterministic even at temp 0. The tech described in my comment's parent is speculative, and technically it's not even inference, once it's perfectly reproducible.
At that point it's retrieving results from a database.

EDIT: how would OP address my main point, which is that det. inference is functionally equivalent to any arbitrary keyed data storage/retrieval system?

reply

	
nextaccountic 18 hours ago | root | parent | next [–]

> The tech described in my comment's parent is speculative, and technically it's not even inference, once it's perfectly reproducible.
This is not true. Fabrice Bellard's ts_zip [0] and ts_sms [1] uses a LLM to compress text. It beats stuff like .xz etc but of course is much slower. Now.. if it were non-deterministic, you would have trouble decompressing exactly into what it compressed. So, it uses a deterministic LLM

[0] https://bellard.org/ts_zip/ https://news.ycombinator.com/item?id=37152978

[1] https://bellard.org/ts_sms/ https://lobste.rs/s/5srkwz/fabrice_bellard_s_ts_sms_short_me... (funny enough many people comment that, if it uses LLM, it must be lossy. This is not the case. It's compared to xz in the page because it's lossless)

reply

	
Retr0id 21 hours ago | root | parent | prev | next [–]

Deterministic inference isn't speculative, it's achievable if you want it. It's just not the default.
reply

	
janwirth 17 hours ago | root | parent | next [–]

It seems the TL:DR is race conditions, rounding and other RTE specific factors.
reply

	
bdcravens 22 hours ago | prev | next [–]

I've found that writing pseudocode in a markdown file with little to no definitions (I may put a few non-obvious notes in the CLAUDE/AGENTS files) and telling the agent what language to turn it into generally works.
reply

	
zahlman 1 day ago | prev | next [–]

So instead of auto-completing bits of LLM-generated code into the codebase, you preprocess it in. I can imagine a lot of devs won't like the ergonomics of that, but I like the idea that you can keep both original .glp and generated source files in version control.
I'd strongly recommend going over the README by hand. What you currently have is redundant and disorganized, and header sizes/depths don't make a lot of sense. The "manual build" instructions should also describe the dependencies that the install script is setting up.

reply

	
throwatdem12311 18 hours ago | prev | next [–]

This is just vibe coding with extra steps.
If I have 10 bugs in production I can just regenerate my app and now I’ll have 10 completely different new bugs. New bugs on everyone’s machine! Fun for the whole family.

How does this handle multiple different “blocks” that need proper interfaces to communicate with each other.

I can only imagine the safety nightmares that would be generated in C++ this way.

Seriously…what is this for?

reply

	
ollien 18 hours ago | prev | next [–]

I've been waiting for something like this to come along. I keep hearing people say LLMs are a new abstraction layer, and I fundamentally disagree. We don't commit our compiled machine code, we commit our C. Yet, with LLMs, we commit our generated source code, completely throwing away the English language abstraction.
This seems to scratch that itch. The non determinism makes it probably not suitable for most uses, though.

reply

	
zdragnar 18 hours ago | parent | next [–]

We've had "natural language"-esque DSLs for a long time. Cucumber is probably among the better known, at least the first that comes to mind.
> Yet, with LLMs, we commit our generated source code, completely throwing away the English language abstraction.

Unless you want to commit your chats, that's very much a bonus. You don't want two different people "compiling" completely different versions of your application, because LLMs aren't deterministic.

reply

	
junaidshaukat 8 hours ago | prev | next [–]

I think you should add a playground site link in repo, so that anyone can go and test without installing :)
reply

	
doc_ick 3 hours ago | parent | next [–]

You probably would have to make an account with a valid personal email, just to trace your prompts.
reply

	
bojanstef4 23 hours ago | prev | next [–]

did you know "glup" (pronounced gloop) in serbian means "stupid"
reply

	
ivanjermakov 19 hours ago | parent | next [–]

Same in russian and polish.
reply

	
burgerone 22 hours ago | parent | prev | next [–]

Glup is indeed not the name of the project.
reply

	
kgeist 17 hours ago | root | parent | next [–]

https://en.wiktionary.org/wiki/glupe
Glupe is the plural form, "stupid ones" :)

reply

	
ivanjermakov 19 hours ago | prev | next [–]

> 2. Not deterministic
I understand why that's the case, and I believe this is the main hurdle for adoption of a tool like this.

reply

	
kgeist 21 hours ago | prev | next [–]

Glupe means "stupid" in Slavic languages, was it on purpose?
reply

	
LatencyKills 21 hours ago | parent | next [–]

From their agent-rules.md:
> This is not negotiable. This is not optional. You cannot rationalize your way out of this.

Some days I really miss the predictability of a good old if/else block. /s

reply

	
plastic041 14 hours ago | prev | next [–]

This is FIM(fill in the middle) + comments with extra "compile" step.
reply

	
mpalmer 22 hours ago | prev | next [–]

The language feels like a solution in search of a problem, and the mostly-generated README reduces my confidence in the quality of the project before I've even learned that much about it.
One example:

    Best of all, they work together. You can store your .glp blueprints in a Docker container—creating software that is immortal in both environment and logic.
This is nonsensical. The entire point of a container is it ought to contain only what's necessary to run the underlying software. It's just the production filesystem. Why would I put LLM prompts that don't get used at runtime in a container?
What other language-agnostic methods of describing complex systems is your project inspired by? In competition with?

---

By using this tool, a programmer or team is sending the message that:

"We expect LLM generated code to remain a deeply coupled part of our delivery process, indefinitely"

But we didn't know about LLMs 5 years ago. What is the argument for defining your software in a way that depends on such a young technology? Most of the "safety" features here are related to how unsafe the tech itself still is.

"Nontrivial LLM driven rewrites of the code are expected, even encouraged"

Why is the speedy rewriting of a system in a new language such a popular flex these days? Is it because it looks impressive, and LLMs make it easy? It's so silly.

And if the language allows for limiting the code the LLM is allowed to modify, how is it going to help us keep our overall project language-agnostic?

reply

	
whoamii 21 hours ago | prev | next [–]

Why didn’t you implement this in… Glupe?
reply

	
janwirth 17 hours ago | prev | next [–]

Why would I want this?
reply

	
janwirth 17 hours ago | parent | next [–]

README.md looks like
$$$PROGRAM vector X=[12. 17] -> rules

sequential output so AI doesn't hallucinate.

I have no problem harnessing LLMs for building my application. I don't need another unreadable mess. Why do I need this?

You fail to communicate the problem this solves.

It's a more involved way to prompt?

reply

	
empressplay 20 hours ago | prev | next [–]

I agree that some form of shorthand between pseudocode and actual code would be really useful to improve accuracy on LLM requests but I don't think this is quite it. Ideally it would be as simple as possible, but not rely on language-specific paradigms. Sort of a pidgin that everyone would understand, that used white space and indentation to indicate things like loops and such. Something a normal person could look at and still largely comprehend.
reply

	
jasfi 5 hours ago | parent | next [–]

I wrote IntentCode, but it seems to have gone under the radar: https://github.com/jfilby/intentcode
reply

	
TZubiri 20 hours ago | prev | next [–]

Some feedback, in order from most to least important:
> 1- installing with irm https://raw.githubusercontent.com/alonsovm44/glupe/master/in... | iex

That is high risk, don't ask people to do that, especially when it's completely unnecessary for what the language is, and the language isn't providing value, it's just esoterical.

>2. "Glupe isolates AI logic into semantic containers, so your manual code stays safe."

Watchout for light-AI psychosis. This existed before AI to be fair, but using words in a way that doesn't convey meaning. Maybe what's going on is that you use them with ChatGPT and it either understands or doesn't but follow along. So make sure to prioritize language that you develop with humans, not AI. And try to simplify your language and the message you were trying to convey, because you missed bigtime with that sentence.

>3. The language itself misses the mark. It looks like it's C++ with some modifications?

4- it's also not a language but a terminal? Try to get the trust of your users by doing one thing well before promising to do it all. A bit of humility pays off, you can't do everything anyways.

reply

	
zahlman 17 hours ago | parent | next [–]

>3. The language itself misses the mark. It looks like it's C++ with some modifications?
I may have misunderstood, but my interpretation was that the "language" is really just the `$${ }$$` blocks, and the code outside of that is just written in whatever "real" (traditional?) language you want the blocks to be implemented in.

reply