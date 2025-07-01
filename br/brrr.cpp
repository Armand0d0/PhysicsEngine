for(int j=0;j<gs->DCcount;j++){
            dLambda[j] = (-distConstraint(gs,j)- alphaTilde*lambda[j]) /
            (2*pow(distance(distConstraintGrad(gs,j,gs->DCs[j].x),ZERO),2.) + alphaTilde);
            lambda[j] += dLambda[j];
        }
        for(int i=0;i<gs->pointMassCount;i++){
            dx[i] = ZERO;
            for(int j=0;j<gs->DCcount;j++){
                dx[i]+= dLambda[j]*distConstraintGrad(gs,j, i);
            }
            gs->dynamicPos[i] += dx[i];
            
        }


for(int j=0;j<gs->DCcount;j++){
            dLambda[j] = (-distConstraint(gs,j)- alphaTilde*lambda[j]) /
            (2*pow(distance(distConstraintGrad(gs,j,gs->DCs[j].x),ZERO),2.) + alphaTilde);

            for(int i=0;i<gs->pointMassCount;i++){
                dx[i] = ZERO;
                for(int j=0;j<gs->DCcount;j++){
                    dx[i]+= dLambda[j]*distConstraintGrad(gs,j, i);
                }  
            }
            lambda[j] += dLambda[j];
            for(int i=0;i<gs->pointMassCount;i++){
                gs->dynamicPos[i] += dx[i];
            }
        }

for(int i=0;i<gs->pointMassCount;i++){
            dx[i] = ZERO;
            for(int j=0;j<gs->DCcount;j++){
                dLambda[j] = (-distConstraint(gs,j)- alphaTilde*lambda[j]) /
                (2*pow(distance(distConstraintGrad(gs,j,gs->DCs[j].x),ZERO),2.) + alphaTilde);
                lambda[j] += dLambda[j];
                dx[i]+= dLambda[j]*distConstraintGrad(gs,j, i);
            }
            gs->dynamicPos[i] += dx[i];
        }