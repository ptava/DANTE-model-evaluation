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

// Get last cell width
inline scalar lastCellSize
(
    const scalar d0,
    const scalar r,
    const int N
)
{
    if (N <= 1)
    {
        return d0;
    }
    return d0 * pow(r, N - 1);
}

inline scalar effectiveExpRatio
(
    const scalar cellToCellRatio,
    const scalar endToStartExpRatio
)
{
    if (endToStartExpRatio <= 0)
    {
        FatalErrorInFunction
            << "Invalid expansion ratio: endToStartExpRatio=" << endToStartExpRatio << nl
            << abort(FatalError);
    }

    return (endToStartExpRatio >= 1.0 ? cellToCellRatio: 1.0/cellToCellRatio);
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
    return exp((N-1)*log(r));
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
    // os.precision(15);
    // Info.stream().precision(15);
    os << "nc" << tag << token::SPACE << nCells << token::END_STATEMENT << endl;
    os << "g"  << tag << token::SPACE << grading << token::END_STATEMENT << endl;
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
    // os.precision(15);
    // Info.stream().precision(15);
    os << "nc" << tag << token::SPACE << nCells << token::END_STATEMENT << endl;
    os << "g"  << tag << token::SPACE << "(";
    forAll(grading, i)
    {
        os.precision(15);
        os << grading[i] << token::SPACE;
    }
    os << ")" << token::END_STATEMENT << endl;
}

inline scalar solve_r_from_L_d0_N
(
    scalar L,
    scalar d0,
    int N,
    int maxIter = 10000,
    scalar rStart = 1.0 + 1e-6
)
{
    if (N <= 1) return 1.0;
    scalar r = rStart;
    for (int it=0; it<maxIter; ++it)
    {
        scalar f   = d0 * (1.0 - pow(r, N))/(1.0 - r) - L;
        scalar df  =
            d0 * ((N-1) * pow(r, N+1) - N * pow(r, N) + r)
            / (sqr(1.0 - r) * r);
        r = max(SMALL, r - (f/df));

        scalar residual = mag(d0 * (1.0 - pow(r, N))/(1.0 - r) - L);
        if (residual < SMALL) break;
    }
    return r;
}

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
    int N = round(L / min(d0, dN));
    scalar currentTotExpRatio = 1.0;

    // Return uniform grading if requested
    if (mag(dN - d0) <= SMALL)
    {
        return {N, currentTotExpRatio};
    }

    for (; N > 2; N--)
    {
        scalar r = solve_r_from_L_d0_N(L, d0, N);
        scalar lastCell = lastCellSize(d0, r, N);

        if (lastCell > dN)
        {
            N++;

            // Make sure N is even
            if (N % 2 != 0) N++;

            // Recompute currentTotExpRatio
            currentTotExpRatio = endToStartExpRatio(
                N,
                solve_r_from_L_d0_N(L, d0, N)
            );
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
        InfoInFunction
            << "For edge " << edgeTag << ", cell-to-cell ratio "
            << rPlanned << " differs from user-defined " << rUser << nl
            << endl;
    }
    if (rUser - rPlanned < 0)
    {
        FatalErrorInFunction
            << "Could not meet edge discratization requirements ("
            << edgeTag << ")." << nl
            << "Try to: " << nl
            << "- change start/final cells width" << nl
            << "- increase grid scaling parameter" << nl
            << "- increase cell-to-cell ratio up to " << rPlanned
            << abort(FatalError);
    }
}

} // namespace Foam
