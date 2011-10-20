/*---------------------------------------------------------------------------*\
  =========                 |
  \\      /  F ield         | OpenFOAM: The Open Source CFD Toolbox
   \\    /   O peration     |
    \\  /    A nd           | Copyright (C) 1991-2010 OpenCFD Ltd.
     \\/     M anipulation  |
-------------------------------------------------------------------------------
License
    This file is part of OpenFOAM.

    OpenFOAM is free software: you can redistribute it and/or modify it
    under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    OpenFOAM is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
    for more details.

    You should have received a copy of the GNU General Public License
    along with OpenFOAM.  If not, see <http://www.gnu.org/licenses/>.

Class
    Foam::mcReactionModel

Description
    Abstract base class for Monte Carlo reaction models

SourceFiles
    mcReactionModel.C

Author
    Michael Wild

\*---------------------------------------------------------------------------*/

#ifndef mcReactionModel_H
#define mcReactionModel_H

#include "autoPtr.H"
#include "dictionary.H"
#include "runTimeSelectionTables.H"
#include "typeInfo.H"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace Foam
{

class mcParticle;
class mcParticleCloud;

/*---------------------------------------------------------------------------*\
                       Class mcReactionModel Declaration
\*---------------------------------------------------------------------------*/

class mcReactionModel :
    public dictionary
{

    // Private Member Functions

        // Disallow default bitwise copy construct and assignment
        mcReactionModel(const mcReactionModel&);
        void operator=(const mcReactionModel&);

public:

    //- Runtime type information
    TypeName("mcReactionModel");


    // Declare run-time constructor selection table

        declareRunTimeSelectionTable
        (
            autoPtr,
            mcReactionModel,
            mcReactionModel,
            (
                const dictionary& mcReactionModelDict
            ),
            (mcReactionModelDict)
        );

    // Constructors

        //- Construct from components
        mcReactionModel
        (
            const dictionary& mcReactionModelDict
        );


    // Selectors

        //- Return a reference to the selected reaction model
        static autoPtr<mcReactionModel> New
        (
            const dictionary& mcReactionModelDict
        );

    //- Destructor
        virtual ~mcReactionModel()
        {}

    // Member Functions

        //- Apply the reaction model to the particle cloud
        virtual void correct(mcParticleCloud& cloud) = 0;

        //- Apply the reaction model to a single particle
        virtual void correct
        (
            mcParticleCloud& cloud,
            mcParticle& p
        ) = 0;

};

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace Foam

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //