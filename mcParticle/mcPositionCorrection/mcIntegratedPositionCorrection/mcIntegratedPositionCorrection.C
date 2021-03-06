/*---------------------------------------------------------------------------*\
                pdfFoam: General Purpose PDF Solution Algorithm
                   for Reactive Flow Simulations in OpenFOAM

 Copyright (C) 2012 Michael Wild, Heng Xiao, Patrick Jenny,
                    Institute of Fluid Dynamics, ETH Zurich
-------------------------------------------------------------------------------
License
    This file is part of pdfFoam.

    pdfFoam is free software: you can redistribute it and/or modify it
    under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 2 of the License, or
    (at your option) version 3 of the same License.

    pdfFoam is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
    for more details.

    You should have received a copy of the GNU General Public License
    along with pdfFoam.  If not, see <http://www.gnu.org/licenses/>.

\*---------------------------------------------------------------------------*/

#include "mcIntegratedPositionCorrection.H"

#include "addToRunTimeSelectionTable.H"
#include "fvCFD.H"
#include "interpolation.H"
#include "mcParticleCloud.H"

// * * * * * * * * * * * * * * Static Data Members * * * * * * * * * * * * * //

namespace Foam
{

    defineTypeNameAndDebug(mcIntegratedPositionCorrection, 0);
    addNamedToRunTimeSelectionTable
    (
        mcPositionCorrection,
        mcIntegratedPositionCorrection,
        mcPositionCorrection,
        integrated
    );

} // namespace Foam

// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

Foam::mcIntegratedPositionCorrection::
mcIntegratedPositionCorrection
(
    mcParticleCloud& cloud,
    const objectRegistry& db,
    const word& subDictName
)
:
    mcPositionCorrection(cloud, db, subDictName),

    pPosCorr_
    (
        IOobject
        (
            thermoDict().lookupOrDefault<word>("pPosCorrName", "pPosCorr"),
            db.time().timeName(),
            db,
            IOobject::MUST_READ,
            IOobject::AUTO_WRITE
        ),
        cloud.mesh()
    ),

    UPosCorr_
    (
        IOobject
        (
            thermoDict().lookupOrDefault<word>("UPosCorrName", "UPosCorr"),
            db.time().timeName(),
            db,
            IOobject::MUST_READ,
            IOobject::AUTO_WRITE
        ),
        cloud.mesh()
    )
{
    setRefCell
    (
        pPosCorr_,
        cloud.mesh().solutionDict().subDict("SIMPLE"),
        pRefCell_,
        pRefValue_
    );
}

// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //

void Foam::mcIntegratedPositionCorrection::updateInternals()
{
    const scalar C = solutionDict().lookupOrDefault<scalar>("C", 1e-5);
    const dimensionedScalar& deltaT = cloud().deltaT();
    const volScalarField& rho = cloud().rhocPdf();
    const volScalarField& pnd = cloud().pndcPdfInst();

    volScalarField rhs = rho - pnd;
    // correction factor to make the RHS of the Poisson equation integrate
    // to zero
    dimensionedScalar beta =
        fvc::domainIntegrate(rhs) / gSum(cloud().mesh().V());
    beta.dimensions() /= dimVolume;

    // solve Poisson equation for pPosCorr
    fvScalarMatrix pPosCorrEqn
    (
        fvm::laplacian(pPosCorr_) == C/sqr(deltaT)*(rhs - beta)
    );
    pPosCorrEqn.setReference(pRefCell_, pRefValue_);
    pPosCorrEqn.relax();
    pPosCorrEqn.solve();

    // time-integrator for correction velocity
    solve(fvm::ddt(cloud().pndcPdf(), UPosCorr_) == -fvc::grad(pPosCorr_));

    UPosCorrInterp_ = interpolation<vector>::New
    (
        cloud().solutionDict().interpolationScheme(UPosCorr_.name()),
        UPosCorr_
    );
}


void Foam::mcIntegratedPositionCorrection::correct(mcParticle& part)
{
    const point& p = part.position();
    label c = part.cell();
    label f = part.face();
    part.Ucorrection() += UPosCorrInterp_().interpolate(p, c, f);
}

// ************************************************************************* //
