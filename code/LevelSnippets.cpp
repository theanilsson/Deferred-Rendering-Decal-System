// This snippet is part of the level loading logic
{
	std::vector<int> decalBoxIds = GetIdFromTag(aLevelData.tags, "DecalBox");
	for(auto& decalBoxId : decalBoxIds)
	{
		std::shared_ptr<DeferredDecal> decal = std::make_shared<DeferredDecal>();
		decal->SetTransform(aLevelData.transforms[decalBoxId]);
		decal->Init(aLevelData.decalPaths[decalBoxId]);
		myDeferredDecals.push_back(decal);
	}
}

// This snippet is part of the level render logic, making the Decal drawcalls
{
	aGeometryBuffer.SetAsActiveTarget(DE::GeometryBuffer::eGeometryBufferTexture::Albedo);
	aGeometryBuffer.SetAsResourceOnSlot(DE::GeometryBuffer::eGeometryBufferTexture::WorldPosition, 6);
	auto& graphicsStateStack = graphicsEngine.GetGraphicsStateStack();
	graphicsStateStack.Push();
	graphicsStateStack.SetCamera(*MainSingleton::GetInstance()->GetActiveCamera());
	graphicsStateStack.SetRasterizerState(DE::RasterizerState::FrontfaceCulling);
	graphicsStateStack.SetDepthStencilState(DE::DepthStencilState::ReadOnlyGreater);
	graphicsStateStack.SetBlendState(DE::BlendState::AlphaBlend);
	graphicsStateStack.UpdateGpuStates();
	for (auto& decal : myDeferredDecals)
	{
		decal->Render(graphicsEngine);
	}
	for (auto& gooPuddle : myGooPuddleDecals)
	{
		gooPuddle->Render(graphicsEngine);
	}
	graphicsStateStack.Pop();
}



// The snippets below here are part of the level's observer pattern recieve logic, triggered when goo is shot from the player's gun
// They were co-written with another programmer in my group but I included them here because the goo uses my decal rendering logic
{
	case eMessageType::CreateGooPuddle:
	{
		std::pair<physx::PxRigidActor*, int>* gooBodyAndWIP = static_cast<std::pair<physx::PxRigidActor*, int>*>(aMessage.messageData);
		physx::PxRigidActor* gooBody = gooBodyAndWIP->first;
		physx::PxVec3 gooPosition = gooBody->getGlobalPose().p;
		DE::Vector3f convertedGooPosition(gooPosition.x, gooPosition.y, gooPosition.z);

		std::shared_ptr<GooPuddle> gooPuddle = std::make_shared<GooPuddle>();
		gooPuddle->Init(convertedGooPosition);
		myGooPuddleDecals.push_back(gooPuddle);
		break;
	}
}

void GooPuddle::Init(DE::Vector3f aPosition)
{
	MainSingleton::GetInstance()->GetAudioManager().StopAudio(eAudioEvent::PlayerGooSplat);
	MainSingleton::GetInstance()->GetAudioManager().PlayAudio(eAudioEvent::PlayerGooSplat);

	float upgradeLevel = MainSingleton::GetInstance()->GetPlayer()->GetGun().GetAmmoUpgradeLevel(eAmmoType::Goo);
	myDurationTimer = CU::CountdownTimer(myDuration * std::pow(2, upgradeLevel - 1));
	myDurationTimer.Reset();
	myBoundingBox = std::make_shared<DE::ModelInstance>(DE::ModelFactory::GetInstance().GetModelInstance(DE::Settings::ResolveAssetPathW(L"3D/cubePrimitive.fbx")));
	myTexture = DE::Engine::GetInstance()->GetTextureManager().GetTexture(L"2D/gloo.png");
	myBoundingBox->SetScale(DE::Vector3f(myRadius * std::pow(2, upgradeLevel - 1)));

	physx::PxVec3 origin = physx::PxVec3(aPosition.x, aPosition.y, aPosition.z);
	physx::PxVec3 direction(0.0f, -1.0f, 0.0f);
	float maxDistance = 80.0f;
	physx::PxQueryFilterData queryFilterData;
	queryFilterData.data.word0 = MainSingleton::GetInstance()->GetCollisionFiltering().Environment;

	physx::PxRaycastBuffer hit;
	if (MainSingleton::GetInstance()->GetPhysXScene()->raycast(origin, direction, maxDistance, hit, physx::PxHitFlag::eDEFAULT, queryFilterData)) // If hit something
	{
		auto hitPoint = DE::Vector3f(hit.block.position.x, hit.block.position.y, hit.block.position.z);

		myBoundingBox->SetLocation(hitPoint);
		myBoundingBox->SetRotation(DE::Rotator(0.0f, 0.0f, 0.0f));

		DE::Vector3f up = DE::Vector3f(0.0f, 0.0f, 0.0f);
		DE::Vector3f forward = { 0.0f, 1.0f, 0.0f };
	
		forward = (forward - forward.Dot(up) * up).GetNormalized();
		DE::Vector3f right = up.Cross(forward);

		DE::Quatf q = DE::Quatf::CreateFromOrthonormalBasisVectors(right, up, forward);
		DE::Vector3f rotation = q.GetEulerAnglesDegrees();
		myBoundingBox->SetRotation(DE::Rotator(rotation.x, UtilityFunctions::GetRandomFloat(0.0f, 360.0f), rotation.z));
	}
}
