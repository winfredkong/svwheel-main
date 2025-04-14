# A computational solution to the stardew valley wheel problem

## Introduction

This project is interested in computing the optimal betting strategy for a problem motivated by the stardew valley fair wheel. 
A generalised formulation of the problem is as follows: 

Fix an intial starting amount of currency $x_0 \in \mathbb{N}$, a target amount $`x^* \in \mathbb{N}`$ and the time cost for a restart $T>0$. 
Every round, the player can either bet some integer amount of his money (say $y < x_0$) or choose to spend $T$ time to restart from $x_0$.
If the player chooses to bet, than either he gains $y$ with probability $p>\frac{1}{2}$ or loses $y$ with probability $1-p$. 

The player now wants to find the optimal betting strategy which minimises the expected hitting time to $`x^*`$.

## Motivation on Formulation

So far much of known discussion on this problem has suggested Kelly's criterion as the optimal solution. 
However, the currency obtained in the fair is only useful in purchasing items from a shop with limited stock. 
This means that currency obtained provides no utility past a certain point. 

In particular, consider that if we want $`x^*=5000`$ to buy out the shop, and our current amount is 4999. 
It makes no sense to bet more than 1 as Kelly criterion would suggest. 
So in fact the betting strategy should never be more than $`x^* - x`$ (where $x$ is the current amount).

Furthermore, from a time optimisation perspective, it makes sense to also take into account the possibility of restarting 
if our currency balance is too low.

## Literature Review

Optimal betting size for recurring games is a well discussed area but they often seek to optimise an increasing utility function of wealth, or do not deal with discrete bet sizes. 
For example, the Kelly Criterion optimises log wealth and allows arbitrarily small bet sizes.
https://sites.oxy.edu/lengyel/M372/papers/gottleib2.pdf looks at optimising expected hitting time but uses a diffusion approximation solution, does not have a discrete bet size and
does not allow for restarting.

## Methodology

Given the Markov nature of the problem (only the current state affects the markov chain), all optimal strategies can be uniquely characterised by optimal action at wealth $x$. 
The set of actions at $x$ is then $\{R, 1, 2, ..., x\}$ (where R represents a reset, and all other positive integers represent bet size).

At this point, there are a few observations we can use to reduce the number of strategies to search:

- If $x \geq x_0$, resetting is suboptimal
- It suffices to consider optimal strategies where we bet no more than $`x^* - x`$
- There exists $x_R$ and $x_A$ where $x_R$ is the largest wealth such that resetting is a optimal strategy, 
and $x_A$ is the smallest wealth where betting $`x^*-x`$ is an optimal strategy. Then, there is an optimal strategy where
it is optimal to reset when wealth $\leq x_R$ and bet $`x^*-x`$ when wealth is $\geq x_A$.
- $\forall x > x_R$, the bet sizes $\{a : x-1 \leq a \leq x-x_R\}$ are weakly dominated by betting $x$. So we need not consider those bet sizes

Some of these observations are non-trivial and the proofs are in [Proofs](#proofs). 

### Naive Search

In searching for an optimal strategy, a naive approach is to  iterate through different possible values of
$`0 \leq x_R < x_0; \frac{x^*}{2} \leq x_A \leq x^*-1`$. Then denote each strategy as some function $`f: \{0,1,2, ..., x^*-1\} \rightarrow \{R, 1, 2, ..., x^*-1\}`$. 
This yields $`x^*`$ linear equations with $`x^*`$ unknowns of the form:

```math
E[S_{x}^{x^*}] =\begin{cases}
      p E[S_{x + f(x)}^{x^*}]+ (1-p)E[S_{x - f(x)}^{x^*}] + 1, & \text{if }f(x)\neq R\\
      E[S_{x_0}^{x^*}] + T, & \text{else}
    \end{cases} 
```

Note that the above set of equations has a unique solution since our last equation is
$`E[S_{x^*-1}^{x^*}] = (1-p)E[S_{x^*-2}^{x^*}] + 1`$, we can then iteratively sub out/remove
$`E[S_{x^*-2}^{x^*}], E[S_{x^*-3}^{x^*}]`$ and so on. The objective function to optimise for is then $`E[S_{x_0}^{x^*}]`$. 

Clearly this is an expensive algorithm that does not scale well since the number of solutions we have to evaluate scales exponentially with $`x^*`$ and each evaluation costs $`O({x^*}^3)`$.

### Value Iteration

Given that our problem is a discrete-time finite Markov decision process,
a better way to solve this is use a value iteration approach in Dynamic Programming. A proof of convergence can be found at https://arxiv.org/pdf/2009.11403

## Proofs

### Lemma 1.1

**Lemma 1.1** Let $`S_{x}^{x^*}`$ be the hitting time of $`x^*`$ starting from $x$ with optimal strategy.
Then $`E[S_{x}^{x^*}]`$  is a non-increasing function of $x$.

**Proof:** Consider that if we start at a higher wealth $x+a$, we can set aside $a$ and play as though we have $x$.
The result follows immediately.

### Lemma 1.2

**Lemma 1.2:** $`\forall x> \frac{x^*}{2}`$, the bet size $`x^* -x`$ weakly dominates (is a better or equal strategy) all larger bet sizes.

**Proof:** If $`x^* -x < a \leq x `$, then from lemma 1.1 the strategy to bet $a$ has expected hitting time

```math
\begin{aligned}
p E[S_{x + a}^{x^*}] + (1-p)E[S_{x -a}^{x^*}] + 1  
&= (1-p)E[S_{x -a}^{x^*}] + 1\\
&\leq (1-p)E[S_{2x - x^*}^{x^*}] + 1 \\
&=  p E[S_{x + x^* - x}^{x^*}] + (1-p)E[S_{2x' - x^*}^{x^*}] + 1 \\
\end{aligned}
```

So bet size $`x^* -x`$ weakly dominates all such $a$.

### Lemma 1.3

**Lemma 1.3:** $\forall x \leq x_0$, resetting is suboptimal.

**Proof:** Suppose otherwise, then we have a contradiction below

```math
E[S_{x}^{x^*}] = E[S_{x_0}^{X^*}] + T \leq  E[S_{x_0}^{X^*}]
```

where the inequality above follows from lemma 1.1.

### Lemma 2.1

**Lemma 2.1:** Suppose that one of the optimal strategies at $x'$ is to bet $`x^* - x'`$, then $`\forall  x' \leq x < x^*,`$ one of the optimal is $`x^* - x`$.

**Proof:** We show that this is true for $x'+1$, then by induction we are done.
Suppose otherwise that optimal for $x'+1$ is $`a < x^* - x' -1`$ (and betting $`x^* - x' -1`$ is not one of the optimal), then we have
 
```math
\begin{aligned}
E[S_{x'+1}^{x^*}] &= p E[S_{x'+1 + a}^{x^*}]+ (1-p)E[S_{x'+1 -a}^{x^*}] + 1 \\ 
&< (1-p)E[S_{2x'+2 - x^*}^{x^*}] + 1 \\
&\leq (1-p)E[S_{2x' - x^*}^{x^*}] + 1 \\
&= E[S_{x'}^{x^*}] 
\end{aligned}
```

Which contradicts Lemma 1.1 so we are done.

### Lemma 2.2 

**Lemma 2.2:** Suppose one of the optimal strategy at $x'$ is to reset, then $\forall  0 \leq x \leq x',$ one of the optimal strategy is to reset.

**Proof:** We show that this is true for $x'-1$, then by induction we are done.

Suppose otherwise that optimal for $x'-1$ is $a \leq x'-1$ and resetting is not optimal, then we have

```math
\begin{aligned}
E[S_{x'-1}^{x^*}] &= p E[S_{x'-1 + a}^{x^*}]+ (1-p)E[S_{x'-1 -a}^{x^*}] + 1 \\ 
&< E[S_{x_0}^{x^*}] + T \\
&= E[S_{x'}^{x^*}] \\
\end{aligned}
```

Which contradicts Lemma 1.1 so we are done.

### Lemma 2.3

**Lemma 2.3:** Let $x_R$ be the largest wealth such that at least one of the optimal strategies is to reset.
Then $\forall x > x_R$, the bet sizes $\{a : x-1 \leq a \leq x-x_R\}$ are weakly dominated by betting $x$.

**Proof**: From Lemma 2.2, we know that $\forall y \leq x_R$, one of the optimal strategies is to reset.
Then, for a bet size $a$ in the set above

```math
\begin{aligned}
p E[S_{x + a}^{x^*}]+ (1-p)E[S_{x -a}^{x^*}] + 1 
&= p E[S_{x + a}^{x^*}]+ (1-p)(E[S_{x_0}^{x^*}] + T)+ 1 \\
&= p E[S_{x + a}^{x^*}]+ (1-p)(E[S_{0}^{x^*}]) + 1 \\
&<= p E[S_{2x_R + 2}^{x^*}]+ (1-p)(E[S_{0}^{x^*}]) + 1 
\end{aligned}
```

So the bet size $x$ weakly dominates $\{a : x-1 \leq a \leq x-x_R\}$.

### Corollary 2.4

**Corollary 2.4:** Let $x_R$ be the largest wealth such that at least one of the optimal strategies is to reset,
$x_A$ be the smallest wealth where betting $`x^*-x`$ is an optimal strategy. If $`x_R \geq \frac{x^*}{2}-1`$,
then an optimal strategy is to reset if $\leq x_R$ and bet $`x^*-x`$ if $> x_R$.

**Proof**: First note that an immediate consequence of Lemma 2.3 is that at wealth $x_R+1$, betting all $x_R+1$ is 
one of the optimal strategies. Then, $`x_R+1 \leq \frac{x^*}{2} \implies x_R+1 \leq x^*-(x_R+1)`$. 
Then by Lemma 1.2 and Lemma 2.1, $x_R+1\geq x_A$ and so we have the optimal strategy described above.