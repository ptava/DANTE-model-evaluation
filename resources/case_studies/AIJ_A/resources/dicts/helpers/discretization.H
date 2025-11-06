/*---------------------------------------------------------------------------*\
  =========                 |
  \\      /  F ield         | OpenFOAM: The Open Source CFD Toolbox
   \\    /   O peration     |
    \\  /    A nd           | www.openfoam.com
     \\/     M anipulation  |
-------------------------------------------------------------------------------
    Copyright (C) 2011-2016 OpenFOAM Foundation
    Copyright (C) 2015-2024 OpenCFD Ltd.
\*---------------------------------------------------------------------------*/
namespace Foam
{

// Define effective cell-to-cell ratio
inline scalar effectiveExpRatio
(
    const scalar cellToCellRatio,
    const scalar dStart,
    const scalar dEnd
)
{
    if (dStart <= 0 || dEnd <= 0)
    {
        FatalErrorInFunction
            << "Invalid cell sizes: dStart=" << dStart << " dEnd=" << dEnd << nl
            << abort(FatalError);
    }
    
    return (dEnd >= dStart ? cellToCellRatio: 1.0/cellToCellRatio);
}

// Return integer N (>=2 when dN!=d0) that best matches L while enforcing
// d0, dN and a constant per-cell ratio. rMax is an optional cap.
inline int cellsForEdge
(
    const scalar L,
    const scalar d0,
    const scalar dN,
    const scalar rMax = GREAT,
    const int    nMax = 20000
)
{
    if (L <= 0 || d0 <= 0 || dN <= 0)
    {
        FatalErrorInFunction
            << "Invalid inputs: L=" << L << " d0=" << d0 << " dN=" << dN << nl
            << abort(FatalError);
    }

    // Uniform case (ends equal): length is d0*N
    if (mag(dN - d0) <= SMALL)
        return max(1, int(round(L/d0)));

    const scalar totExpR = dN/d0;

    int    bestN   = -1;
    scalar bestErr = GREAT;

    for (int N = 2; N <= nMax; ++N)
    {
        const scalar rN = pow(totExpR, 1.0/(N - 1.0));
        if (rMax > 0 && rN > rMax + 1e-12) continue;

        const scalar Lcalc =
            (mag(rN - 1.0) < 1e-12)
            ? d0 * scalar(N)
            : d0 * (pow(rN, scalar(N)) - 1.0)/(rN - 1.0);

        const scalar err = mag(Lcalc - L);
        if (err < bestErr) { bestErr = err; bestN = N; }
        if (err <= SMALL) break; // good enough
    }

    if (bestN < 0)
    {
        FatalErrorInFunction
            << "No integer N satisfies rMax=" << rMax << " for given d0,dN." << nl
            << abort(FatalError);
    }

    return bestN;
}


// Return total expansion ratio over N cells with cell-to-cell ratio r
inline scalar endToStartExpRatio
(
    const int N,
    const scalar r
)
{
    if (N <= 0)
    {
        FatalErrorInFunction
            << "Invalid number of cells N=" << N << nl
            << abort(FatalError);
    }
    if (mag(r - 1.0) < SMALL)
    {
        return 1.0;
    }
    return exp((N - 1)*log(r));
}

// Return cell-to-cell expansion ratio r for given totExpRatio and N
inline scalar cellToCellExpRatio
(
    const int N,
    const scalar totExpRatio
)
{
    if (N <= 1)
    {
        FatalErrorInFunction
            << "Invalid number of cells N=" << N << nl
            << abort(FatalError);
    }
    if (totExpRatio <= 0)
    {
        FatalErrorInFunction
            << "Invalid total expansion ratio=" << totExpRatio << nl
            << abort(FatalError);
    }
    return pow(totExpRatio, 1.0/scalar(N - 1));
}

// Print ncells and grading for blockMeshDict
inline void printBlockInfo
(
    Ostream& os,
    const word& tag,
    const label nCells,
    const scalar grading
)
{
    os << "nc" << tag << token::SPACE << nCells << token::END_STATEMENT << nl;
    os << "g"  << tag << token::SPACE << grading << token::END_STATEMENT << nl;
}

// Print ncells and multi-grading for blockMeshDict
inline void printBlockInfo
(
    Ostream& os,
    const word& tag,
    const label nCells,
    const List<vector>& grading
)
{
    os << "nc" << tag << token::SPACE << nCells << token::END_STATEMENT << nl;
    os << "g"  << tag << token::SPACE << "(";
    forAll(grading, i)
    {
        os << grading[i] << token::SPACE;
    }
    os << ")" << token::END_STATEMENT << nl;
}

// Geometric sum: (1 + r + ... + r^(N-1)) with a safe fallback near r=1
inline scalar geomSum(const scalar r, const int N)
{
    const scalar dr = r - 1.0;
    if (mag(dr) < 1e-8) {
        const scalar Nf = scalar(N);
        const scalar N1 = Nf - 1.0;
        // 2nd-order Taylor: N + 1/2 N(N-1) dr + 1/6 N(N-1)(N-2) dr^2
        return Nf + 0.5*Nf*N1*dr + (Nf*N1*(Nf-2.0))/6.0 * dr*dr;
    }
    return (pow(r, N) - 1.0)/dr;
}

// Solve r from L = d0 * (r^N - 1)/(r - 1)  with N >= 1
inline scalar solve_r_from_L_d0_N(const scalar L, const scalar d0, const int N, const int maxIter=64)
{
    if (N <= 1) return 1.0;                 // single cell: ratio irrelevant
    if (L <= 0 || d0 <= 0) {
        FatalErrorInFunction << "Invalid inputs: L=" << L << " d0=" << d0 << " N=" << N << nl
                             << abort(FatalError);
    }

    const scalar S  = L/d0;                 // target geometric sum
    const scalar Nf = scalar(N);

    // Uniform case: L/d0 == N  => r = 1
    if (mag(S - Nf) <= 1e-12 * max(S, Nf)) return 1.0;

    // Basic sanity: the minimum possible sum with N>=2 is ~1 (as r->0+).
    if (S < 1.0 - 1e-12) {
        FatalErrorInFunction << "Infeasible: L/d0 = " << S << " < 1 for N=" << N << nl
                             << abort(FatalError);
    }

    scalar lo, hi;
    if (S > Nf) {
        // Need r > 1
        lo = 1.0 + 1e-6;
        hi = 2.0;
        // Expand hi until geomSum(hi) >= S (monotone increasing for r>1)
        for (int k=0; k<60 && geomSum(hi, N) < S; ++k) hi *= 2.0;
    } else {
        // Need 0 < r < 1
        lo = 1e-12;
        hi = 1.0 - 1e-6;
        // Ensure lo is low enough so geomSum(lo) <= S (it usually is)
        for (int k=0; k<60 && geomSum(lo, N) > S; ++k) lo *= 0.5;
    }

    // Bisection
    for (int it=0; it<maxIter; ++it) {
        const scalar mid = 0.5*(lo + hi);
        const scalar g   = geomSum(mid, N);

        if (g < S) lo = mid; else hi = mid;

        if (mag(hi - lo) <= 1e-12 * max(1.0, 0.5*(hi+lo))) break;
    }
    return max(0.5*(lo + hi), 1e-12);
}
// Solve r for given (L, d0, N): L = d0 * (r^N - 1)/(r - 1)
// inline scalar solve_r_from_L_d0_N
// (
//     scalar L,
//     scalar d0,
//     int N,
//     int maxIter = 10000
// )
// {
//     if (N <= 1) return 1.0;
//     scalar r = 1.0 + SMALL; // start uniform
//     for (int it=0; it<maxIter; ++it)
//     {
//         scalar rN = pow(r, N);
//         scalar f   = d0 * (1.0 - rN)/(1.0 - r) - L;
//         scalar df  = d0 * ( ((N-1) * pow(r, N+1) - N * rN + r) / (sqr(1.0 - r) * r) );
//         scalar step = f/df;
//         r = max(SMALL, r + step);
//         if (mag(step) < SMALL) break;
//     }
//     return r;
// }

// Plan one edge from a junction with target start size d0, length L, and ratio
// cap rMax (>1). Returns integer N and totExpRatio = r^(N-1)
inline std::tuple<int, scalar> planEdge
(
    scalar L,
    scalar d0,
    scalar dN,
    scalar rMax,
    scalar tol = 1e-4
)
{
    if (rMax < 1.0)
    {
        FatalErrorInFunction
            << "Invalid rMax=" << rMax << " < 1.0" << nl
            << abort(FatalError);
    }

    // Seed: nMax, then decrease until r ≤ rMax
    scalar expectedTotExpRatio = dN / d0;
    int N = round(L / min(d0, dN));
    scalar currentTotExpRatio = 1.0;

    // Return uniform grading if requested
    if (mag(expectedTotExpRatio - currentTotExpRatio) <= SMALL)
    {
        return {N, currentTotExpRatio};
    }

    for (;; --N) {
        scalar r = solve_r_from_L_d0_N(L, d0, N);
        currentTotExpRatio = endToStartExpRatio(N, r);

        // Info << " DEBUG: " 
        //      << " N=" << N
        //      << " r=" << effectiveExpRatio(r, d0, dN)
        //      << " currentTotExpRatio=" << currentTotExpRatio
        //      << " expectedTotExpRatio=" << expectedTotExpRatio
        //      << endl;

        if (currentTotExpRatio > expectedTotExpRatio)
        {
            // Too stretched: need more cells
            N++;
            break;
        }
    }
    return {N, dN / d0 ? currentTotExpRatio : 1.0 / currentTotExpRatio};
}

// Print warning if common ratio differ from the user-defined one
inline void checkCommonRatio
(
    scalar rPlanned,
    scalar rUser,
    const word& edgeTag,
    scalar tol = 1e-4
)
{
    if (mag(rUser - rPlanned) > tol)
    {
        WarningInFunction
            << "For edge " << edgeTag << ", cell-to-cell ratio "
            << rPlanned << " differs from user-defined " << rUser << nl
            << endl;
    }
    if (rUser - rPlanned < 0)
    {
        FatalErrorInFunction
            << "Could not meet both expetected final length "
            << "and max cell-to-cell ratio for edge " << edgeTag << nl
            << "Try to: " << nl
            << "- change start/final cells width" << nl
            << "- increase cell-to-cell ratio up to " << rPlanned
            << abort(FatalError);
    }
}

} // namespace Foam
