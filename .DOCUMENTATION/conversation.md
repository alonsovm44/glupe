Krzysztof's comments:

Hey, in regards to your question. Yggdrasil is more about gathering and aggregation of the knowledge that is around the code but not necessarily in the code itself. When anyone implements some process across many files, there is always a bigger picture. The bigger picture is not captured in the code itself, but it's in the heads of developers, product people and business. The idea behind Yggdrasil is to capture it in a way that's related to the code but without littering the code. In the case of brownfield and reverse engineering, it should be able to help create a bigger picture retroactively from existing source code and the knowledge that can be seeded from other sources. In that case this knowledge plus code gives understanding. Understanding allows unifying. To unify, we can use tools like Glupe if it provides predictable rails on which the unification happens. Infrastructural code is templated and containers are implemented. There's a key difference in how we approach this, though — I'm using tools like Claude Code or Cursor. In the end, such agents gather all the context in a "freestyle" way with usage of the knowledge graph that Yggdrasil provides. Glupe, on the other hand, uses direct API calls, which means agents with Yggdrasil would have to create complete context for Glupe so that it can implement what is requested. Coding agent can implement it itself given the template and the context (but for sure it will not be so solid and can mutate as there are no guardrails regarding areas that are allowed to be edited). When thinking about it, I'm not sure if integration makes sense in such a scenario.

Nice work on adopting the comment markers, by the way, that's a solid DX improvement. I'll check out the variables and functional behavior updates when I get a chance.

Recently I've been researching how this whole concept can be improved. The knowledge for a particular file is assembled by walking the graph upward — parents, grandparents, relations between nodes, plus cross-cutting aspects and flows that describe horizontal paths and rules across the source code.

                         ┌─────────────┐
                         │   Project   │
                         │   Root      │
                         └──────┬──────┘
                    ┌───────────┼───────────┐
                    │           │           │
              ┌─────┴─────┐  ┌──┴───┐ ┌─────┴─────┐
              │  Orders   │  │ Auth │ │ Payments  │
              └─────┬─────┘  └──┬───┘ └─────┬─────┘
                ┌───┼───┐       │       ┌───┼───┐
                │   │   │       │       │       │
              Cart  │  Ship     │    Stripe   Invoice
                 Checkout       │
                                │
· · · · · · · · · · · · · · · · · · · · · · · · · · · · ·  ← Nodes
                                                             (tree)

═══ Aspect: Error Handling ══════════════════════════════  ← Aspects
    applies to: Orders, Payments, Auth                       (horizontal
═══ Aspect: Logging ═════════════════════════════════════    rules across
    applies to: ALL                                          nodes)
═══ Aspect: Retry Policy ════════════════════════════════
    applies to: Payments, Stripe

──→ Flow: Place Order ───────────────────────────────────  ← Flows
    Cart → Checkout → Auth → Stripe → Invoice                (paths through
                                                             nodes)

- - → Checkout ──consumes──→ Stripe - - - - - - - - - - -  ← Relations
      Cart ──consumes──→ Checkout                            (dependencies
      Stripe ──consumes──→ Invoice                           between nodes)

Context package examples (what the agent actually receives):

  Stripe                              Checkout
  ├── Root / responsibility.md        ├── Root / responsibility.md
  ├── Payments / responsibility.md    ├── Orders / responsibility.md
  ├── Payments / interface.md         ├── Orders / interface.md
  ├── Stripe / responsibility.md      ├── Checkout / responsibility.md
  ├── Stripe / logic.md               ├── Checkout / logic.md
  ├── Stripe / interface.md           ├── Checkout / interface.md
  ├── Stripe / errors.md              ├── Checkout / errors.md
  ├── ═ Aspect: Error Handling        ├── ═ Aspect: Error Handling
  ├── ═ Aspect: Retry Policy          ├── ═ Aspect: Logging
  ├──                                 ├── → Flow: Place Order
  └── → Flow: Place Order             └── ⇄ Stripe / interface.md (via relation)
Looking from the detail level up, it's straightforward. You get the full picture for any node. What I'm missing is the top-down path: going from intent down to specific files. Parent nodes are intentionally high-level. They give you the "what" and "why", not the "where exactly in the code." I've been thinking about how to let an agent start from a high-level goal and navigate down to the files it needs to change, using the graph as a map rather than just a context source. Now it has to grep throughout the repository and find the place to start.

I also ran 26 experiments autonomously (master agent with execution subagents, on real open source projects: Hoppscotch, Medusa, Django). Some highlights: an agent with only Yggdrasil context packages and no source code built a correct service scoring 4.93/5.00. Self-calibration converges in two cycles (start messy, enrich, the graph gets good fast). Impact analysis hits 100% recall within the mapped graph. You can check the experiments directory in the repo if you're curious about methodology and detailed results.

You can check the experiments directory in the repo if you're curious. Fair warning: the methodology has limitations I'm honest about. Single-agent scoring, the same codebase for most tests, simulated time travel instead of real aging. The numbers are directional, somewhat helpful. But they were enough to tell me what works and what doesn't. In the future I want to use more reliable infrastructure for that using more independent agents and samples to get better estimates.

You know what? Actually, what I see from the concept is something that I missed when I approached your idea. The way you want a process of code materialization to be done gives something that I have a problem with right now. Experiments that I did have shown that agents are awful at detecting missing aspects in the code. They're good at saying that code contradicts the specification, but it's a lot worse when I ask to check completeness in relation to bold specs. And in relation to that problem, I thought that the substraction you described could be used as a way to detect if there are implemented things that were not specified OR there are things in spec not implemented. So I have an implementation and specification. Both can be somehow reduced to the same form and subtract them both ways. Given the result of that operation agent will know what is not matching expectations and the result will be iteratively better. Right now I just "believe" that agent will implement everything that is requested given how Yggdrasil works. It just provides local expectations and the surroundings.

Conceptually, THIS is actually the use case for Glupe with the possibility of those operations. Now I perceive what I do a little bit too early for current market expectations. Most of the people are just fixing CLAUDE.md and other rules and stating it's fine. But right now there's a big issue starting to be visible in companies. They need more time to review the code, and they believe it's ok rushing through it. And the problem will be even more visible in the future, the code generation will be faster, and acceptance will still stay on humans. And they will not keep up with this. It's just too quick. The older days there were a lot of team refinements, and it made code review easier because everyone understood what the result should look like. But now it's not the case anymore given the pace of code being generated by developers.

What idea do you have on making those operations work? Making it could be really useful.