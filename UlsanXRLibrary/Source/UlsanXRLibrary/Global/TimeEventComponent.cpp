// Fill out your copyright notice in the Description page of Project Settings.


#include "Global/TimeEventComponent.h"

// Sets default values for this component's properties
UTimeEventComponent::UTimeEventComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UTimeEventComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...

}


// Called every frame
void UTimeEventComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (true == Events.IsEmpty())
	{
		return;
	}

	for (size_t i = 0; i < Events.Num(); i++)
	{
		FTimeEvent& Event = Events[i];
		Event.CurTime += DeltaTime;
		if (nullptr != Event.UpdateFunction)
		{
			Event.UpdateFunction(DeltaTime, Event.CurTime);
		}
	}

	// 블루프린트로 
	for (size_t i = 0; i < Events.Num(); i++)
	{
		FTimeEvent& Event = Events[i];
		if (Event.MaxTime < Event.CurTime)
		{
			if (nullptr != Event.EndFunction)
			{
				Event.EndFunction();
			}

			if (false == Event.IsLoop)
			{
				// 5번째 
				Events.RemoveAt(i);
				--i;
			}
			else
			{
				Event.CurTime = 0.0f;
			}
		}

	}
}

void UTimeEventComponent::AddEndEvent(float _Time, TFunction<void()> _EndFunction, bool _IsLoop /*= false*/)
{
	AddEvent(_Time, nullptr, _EndFunction, _IsLoop);
}

void UTimeEventComponent::AddUpdateEvent(float _Time, TFunction<void(float, float)> _UpdateFunction, bool _IsLoop /*= false*/)
{
	AddEvent(_Time, _UpdateFunction, nullptr, _IsLoop);
}

void UTimeEventComponent::AddEvent(float _Time, TFunction<void(float, float)> _UpdateFunction, TFunction<void()> _EndFunction, bool _IsLoop /*= false*/)
{
	FTimeEvent& NewEvent = Events.Emplace_GetRef();
	NewEvent.IsLoop = _IsLoop;
	NewEvent.MaxTime = _Time;
	NewEvent.CurTime = 0.0f;
	NewEvent.EndFunction = _EndFunction;
	NewEvent.UpdateFunction = _UpdateFunction;
}

